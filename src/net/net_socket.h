/***********************************************************************************************
// 文件名:     net_socket.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2016-9-12 11:50:18
// 内容描述:   定义TCP通信的基本类
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_NET_SOCKET_H
#define BASIC_NET_SOCKET_H

/////////////////////////////////////////////////////////////////////////////////////////////
#include "event.h"
#include "evdns.h"
#include "sendbuffer.h"

#ifdef __BASICWINDOWS
#define BASICWINDOWS_USE_IOCP
#endif

__NS_BASIC_START
#define READBUFFERSIZE_MSG			16384
/////////////////////////////////////////////////////////////////////////////////////////////
class CNetThread;
class CBasicSessionNet;
struct CEventQueueItem;
#ifdef BASICWINDOWS_USE_IOCP

/* Mingw's headers don't define LPFN_ACCEPTEX. */
typedef BOOL(WINAPI *AcceptExPtr)(SOCKET, SOCKET, PVOID, DWORD, DWORD, DWORD, LPDWORD, LPOVERLAPPED);
typedef BOOL(WINAPI *ConnectExPtr)(SOCKET, const struct sockaddr *, int, PVOID, DWORD, LPDWORD, LPOVERLAPPED);
typedef void (WINAPI *GetAcceptExSockaddrsPtr)(PVOID, DWORD, DWORD, DWORD, LPSOCKADDR *, LPINT, LPSOCKADDR *, LPINT);

/** Internal use only. Holds pointers to functions that only some versions of
Windows provide.
*/
struct IOCPExt_Func{
	AcceptExPtr AcceptEx;
	ConnectExPtr ConnectEx;
	GetAcceptExSockaddrsPtr GetAcceptExSockaddrs;
};

enum IOType{
	IORead,
	IOWrite,
	IOFunc,
	IOAccept,
	IOConnect,
};

struct OVERLAPPEDPLUS{		//internal class  no derived from ctlobject
	OVERLAPPED		m_ol;
	IOType			m_ioType;
	OVERLAPPEDPLUS(){
		ZeroMemory(this, sizeof(OVERLAPPEDPLUS));
	}
};
#endif

class CBasicNet_Socket{
public:
	typedef void(*pCallSameRefNetSessionFunc)(CBasicNet_Socket* pRefNetSession, intptr_t lRevert);
	static uint32_t GetDefaultCreateSessionID(){ return m_defaultCreateSession.fetch_add(1, memory_order_relaxed); }
public:
	CBasicNet_Socket(uint32_t nSessionID);
	virtual ~CBasicNet_Socket();
public:
	//! 获取socketid
	evutil_socket_t& GetSocketID(){ return m_socketfd; }

	//! 关闭
	void Close(bool bRemote = false, bool bMustClose = false);

	//! 回调中可以安全删除
	virtual void SafeDelete();

	//! 注册过滤器
	int RegistePreSend(CBasicPreSend* pFilter, uint32_t dwRegOptions = 0);

	//! 获取注册的过滤器
	CBasicPreSend* GetPreSend(){ return m_pPreSend; }

	//! 获取真实的session
	virtual CBasicSessionNet* GetRealSessionNet() = 0;

	//! 加入队列
	void AddSocketCallFunc(CEventQueueItem* pItem);

	//! 执行队列
	void RunSocketCallFunc();

	//! 获取netthread
	CNetThread* GetSelfNetThread(){ return m_pThread; }
#ifdef BASICWINDOWS_USE_IOCP
	//! server监听异步accept使用
	virtual void ServerAcceptEx(OVERLAPPEDPLUS* pOverlapPlus){}

	//! client连接使用
	virtual void ClientConnectEx(){}
#endif
public:
	//! 是否连接
	bool IsConnected();
	//! 是否认证成功
	bool IsTransmit();
	//! 判断是否准备close
	bool IsToClose();

	//! 设置准备关闭
	void SetToClose();
	//! 设置安全删除标志
	void SetToSafeDelete();

	//! 判断是否可以关闭
	virtual bool CanClose();

	//! 获取和设置状态
	uint32_t GetSessionStatus(uint32_t dwMask){ return m_unSessionStatus & dwMask; }
	void SetSessionStatus(uint32_t dwValue, uint32_t dwMask){ m_unSessionStatus &= ~dwMask; m_unSessionStatus |= (dwValue & dwMask); }
public:
	//! ontimer线程
	virtual bool OnTimer(unsigned int nTick);
protected:
	//! 发送对应线程消息
	void SetLibEvent(pCallSameRefNetSessionFunc pCallback, intptr_t lRevert = 0);

	//! 重新初始化成员
	virtual void InitMember();

	//! 线程内执行函数
	virtual void CloseCallback(BOOL bRemote, DWORD dwNetCode = 0);
protected:
	static std::atomic<uint32_t>	m_defaultCreateSession;
	CNetThread*						m_pThread;
#ifndef BASICWINDOWS_USE_IOCP
	event							m_revent;
#endif
	evutil_socket_t					m_socketfd;

	CBasicPreSend*					m_pPreSend;

	uint32_t						m_unSessionStatus;				//状态  TIL_SS_*

	//消息队列
	basiclib::SpinLock				m_lockMsg;
	basiclib::CBasicSmartBuffer		m_smBuf;
	basiclib::CBasicSmartBuffer		m_smIOCPBuf;

	friend class CNetThread;
};

#define MAX_BUFFER_SEND_BUF				4096
class CBasicNet_SocketTransfer : public CBasicNet_Socket{
public:
	CBasicNet_SocketTransfer(CBasicSessionNetNotify* pFather, uint32_t nSessionID, uint16_t usRecTimeout = 0);
	virtual ~CBasicNet_SocketTransfer();

	//! 发送数据
	virtual int32_t Send(void *pData, int32_t cbData, uint32_t dwFlag = 0);

	//内部释放
	int32_t SendData(SendBufferCacheMgr& sendData);

	//! 判断是否可以关闭
	virtual bool CanClose();

	//! 判断是否超时没收到数据
	BOOL IsRecTimeout(time_t tmNow, uint16_t nTimeoutSecond);

	//! 获取netstate
	void GetNetStatInfo(BasicNetStat& netState){ netState = m_stNet; }

	//! 获取状态
	virtual void GetNetStatus(CBasicString& strStatus) = 0;
public:
	//! ontimer线程
	virtual bool OnTimer(unsigned int nTick);

	//! onidle
	void OnIdle();
protected:
	friend void OnLinkRead(evutil_socket_t fd, short event, void *arg);
	friend void OnLinkWrite(evutil_socket_t fd, short event, void *arg);

	//! 重新初始化成员
	virtual void InitMember();

	//! 线程内执行函数
	virtual void CloseCallback(BOOL bRemote, DWORD dwNetCode = 0);

	//! 读事件
	void OnReadEvent();

	//! 写事件
	void OnWriteEvent();

	//! 只在libevent线程使用
	void SendDataFromQueue();
#ifdef BASICWINDOWS_USE_IOCP
	//! IOCP回调发送成功字节
	void SendDataSuccessAndCheckSend(DWORD dwIoSize);
#endif

	//! 加入发送队列
	void AddSendQueue(SendBufferCache* pSendCache);

	//! 收到数据
	void OnReceiveData(const char* pszData, uint32_t dwIoSize);

	//! 重置过滤器状态
	void ResetPreSend();

	//! 断开消息
	void OnDisconnect(uint32_t dwNetCode);
	void OnSendData(uint32_t dwIoSize);
	void OnConnect(uint32_t dwNetCode);
	uint32_t OnReceive(uint32_t dwNetCode, const char *pszData, int32_t cbData);
	void OnError(uint32_t dwNetCode, int32_t lRetCode);
	//! 过滤器接受数据
	void PreReceiveData(uint32_t dwNetCode, const char *pszData, int32_t cbData);
protected:
	virtual CBasicSessionNet* GetRealSessionNet(){ return m_pFather; }
protected:
	static unsigned short	m_usTimeoutShakeHandle;		//default shakehandle timeout time
	CBasicSessionNetNotify*	m_pFather;
	uint16_t				m_usRecTimeout;				//超时时间，0代表不超时
	uint32_t				m_unIdleCount;				//进入空闲的次数

	BasicNetStat			m_stNet;
	BasicNetStat			m_lastNet;

	//线程内使用
	CBasicBitstream			m_bufCacheTmp;
#ifdef BASICWINDOWS_USE_IOCP
	bool					m_bSend;
	OVERLAPPEDPLUS			m_olRead;
	OVERLAPPEDPLUS			m_olWrite;
	WSABUF					m_wsaInBuffer;
	char					m_byInBuffer[MAX_BUFFER_SEND_BUF];		//预分配的接收数据缓冲区
	WSABUF					m_wsaOutBuffer;
	char					m_byOutBuffer[MAX_BUFFER_SEND_BUF];		//预分配的发送数据缓冲区
#else
	event					m_wevent;
#endif

private:
	//发送的缓存区
	CMsgSendBuffer			m_msgQueue;

	friend class CNetThread;
};

__NS_BASIC_END
//////////////////////////////////////////////////////////////////////////////////////////////////

#endif 
