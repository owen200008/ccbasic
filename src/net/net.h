/***********************************************************************************************
// 文件名:     net.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2016-9-12 11:50:18
// 内容描述:   定义TCP通信的基本类
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_NET_H
#define BASIC_NET_H

/////////////////////////////////////////////////////////////////////////////////////////////
#include "event.h"
#include "evdns.h"

__NS_BASIC_START
/////////////////////////////////////////////////////////////////////////////////////////////
//声明
//class CBasicObject;
class CBasicSessionNet;				//基类
class CBasicSessionNetServer;		//监听
class CBasicSessionNetClient;		//主动连接

struct BasicNetStat;		//发送接收的统计信息
#pragma	pack(1)

/////////////////////////////////////////////////////////////////////////////////////////////
//返回错误定义
#define NET_ERROR								(0xE0000000 | _ERROR_NET)	    //网络错误

#define BASIC_NET_OK							0								//成功，没有错误

#define BASIC_NET_GENERIC_ERROR					(NET_ERROR | 0x0001)			//一般性错误，即未知的错误
#define BASIC_NET_OUTOF_MAXPACKET				(NET_ERROR | 0x0002)			//超出最大数据包的限制
#define BASIC_NET_NO_MEMORY						(NET_ERROR | 0x0003)			//内存不足
#define BASIC_NET_ADDRESS_ERROR					(NET_ERROR | 0x0004)			//地址错误
#define BASIC_NET_BIND_ERROR					(NET_ERROR | 0x0005)			//bind 函数调用错误
#define BASIC_NET_LISTEN_ERROR					(NET_ERROR | 0x0006)			//listen 函数调用错误
#define BASIC_NET_TOCLOSE_ERROR					(NET_ERROR | 0x0007)			//正在关闭
#define BASIC_NET_CONNECTING_ERROR				(NET_ERROR | 0x0008)			//正在连接
#define BASIC_NET_ALREADY_LISTEN				(NET_ERROR | 0x0009)			//已经监听
#define BASIC_NET_NO_CONNECT					(NET_ERROR | 0x000a)			//没有连接
#define BASIC_NET_ALREADY_CONNECT				(NET_ERROR | 0x000b)			//已经连接
#define BASIC_NET_INVALID_ADDRESS				(NET_ERROR | 0x000c)			//地址非法
#define BASIC_NET_FILTER_WRITE_FAIL				(NET_ERROR | 0x000d)			//过滤器处理 OnWriteData 失败
#define BASIC_NET_FILTER_READ_FAIL				(NET_ERROR | 0x000e)			//过滤器处理 OnReadData  失败
#define BASIC_NET_SOCKET_ERROR					(NET_ERROR | 0x000f)			//socket 函数调用错误
#define BASIC_NET_RELEASE_ERROR					(NET_ERROR | 0x0010)			//RELEASE错误

//返回的状态代码
#define BASIC_NETCODE_SUCC					0x00000001				//成功
#define BASIC_NETCODE_CLOSE_REMOTE			0x00000002				//服务端主动关闭
#define BASIC_NETCODE_FILTER_HANDLE			0x00000004				//过滤器处理过
#define BASIC_NETCODE_FILTER_ERROR			0x00000008				//过滤器处理错误

#define BASIC_NETCODE_CONNET_FAIL			0x00000040				//连接失败

//绑定函数 HandleConnect 返回值
#define BASIC_NET_HC_RET_HANDSHAKE			0x00000001		//连接需要握手过程

//绑定函数 HandleReceive 返回值
#define BASIC_NET_HR_RET_HANDSHAKE			0x00000001		//握手过程成功


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! 过滤器返回值
#define PACK_FILTER_FAIL		-1		/*!<失败*/
#define PACK_FILTER_SKIP		0		/*!<成功，数据未改动*/
#define PACK_FILTER_NEXT		1		/*!<成功，还有数据需要继续处理*/
#define PACK_FILTER_HANDLED		2		/*!<成功，保留数据*/
#define PACK_FILTER_SEARCH		3		/*!<成功，传递给下一个*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*!
* 初始化通信所需的参数。
* \return 返回 BASIC_NET_*
* \remarks 在使用通信类以前必须调用该函数
*/
typedef basiclib::CBasicString(*pGetConfFunc)(const char* pParam);
_BASIC_DLL_API void SetNetInitializeGetParamFunc(pGetConfFunc func);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class _BASIC_DLL_API CBasicPreSend : public CBasicObject
{
public:
	CBasicPreSend() {}

public:
	/*\brief 过滤收到的数据 */
	// 过滤生成的数据放入buf
	virtual Net_Int OnPreReceive(const char *pszData, Net_Int cbData, CBasicBitstream& buf, CBasicSessionNetClient* pNetSession) = 0;

	/*\brief 过滤发送的数据 */
	// 过滤生成的数据放入buf
	virtual Net_Int OnPreSend(const char *pszData, Net_Int cbData, Net_UInt dwFlag, SendDataToSendThread& buf) = 0;

	/*\brief 构造新的实例 */
	// 用于Accept
	virtual CBasicPreSend* Construct() = 0;

	/*\brief 重置过滤器状态 */
	virtual void ResetPreSend() = 0;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! 发送接收的统计信息
/*!
*  用在统计发送接收的字节数和次数。溢出后从零开始。
*/
struct _BASIC_DLL_API  BasicNetStat
{
	Net_UInt	m_dwSendBytes;		/*!< 发送的字节数 */
	Net_UInt	m_dwSendTimes;		/*!< 发送的次数 */
	Net_UInt	m_dwReceBytes;		/*!< 接收的字节数 */
	Net_UInt	m_dwReceTimes;		/*!< 接收的次数 */
	time_t		m_tmLastRecTime;	//记录最后收到数据的时间
	double		m_fLastSendRate;
	double		m_fLastRecvRate;
	time_t		m_tLastStat;
	BasicNetStat()
	{
		Empty();
	}

	void Empty()
	{
		memset(this, 0, sizeof(BasicNetStat));
	}
	void OnSendData(int nSend)
	{
		if (nSend > 0)
		{
			m_dwSendBytes += nSend;
			m_dwSendTimes++;
		}
	}
	void OnReceiveData(int nRece)
	{
		if (nRece > 0)
		{
			m_dwReceBytes += nRece;
			m_dwReceTimes++;
		}
		m_tmLastRecTime = time(NULL);
	}
	void GetTransRate(BasicNetStat& lastData, double& dSend, double& dRecv){
		DWORD tNow = basiclib::BasicGetTickTime();
		if (m_tLastStat > 0 && (tNow - m_tLastStat)  < 10000)
		{
			dSend = m_fLastSendRate;
			dRecv = m_fLastRecvRate;
			return;
		}

		if (m_tLastStat == 0)
		{
			m_tLastStat = tNow;
			lastData = *this;
			return;
		}

		DWORD dwTotalBytes0 = lastData.m_dwReceBytes;
		dwTotalBytes0 += lastData.m_dwSendBytes;
		DWORD dwTotalBytes1 = m_dwReceBytes;
		dwTotalBytes1 += m_dwSendBytes;
		m_fLastSendRate = double(m_dwSendBytes - lastData.m_dwSendBytes) / 1024 / (double(tNow - m_tLastStat) / 1000);
		m_fLastRecvRate = double(m_dwReceBytes - lastData.m_dwReceBytes) / 1024 / (double(tNow - m_tLastStat) / 1000);

		dSend = m_fLastSendRate;
		dRecv = m_fLastRecvRate;

		m_tLastStat = tNow;
		lastData = *this;
	}
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define TIL_SS_LINK						0x0000000F		//连接的状态
#define TIL_SS_IDLE						0x00000000		//空闲
#define TIL_SS_CONNECTING				0x00000001		//正在连接
#define TIL_SS_CONNECTED				0x00000002		//已连接

#define TIL_SS_CLOSE					0x000000F0		//关闭的状态
#define TIL_SS_NORMAL					0x00000000		//正常状态
#define TIL_SS_TOCLOSE					0x00000010		//正在关闭

#define TIL_SS_SHAKEHANDLE_MASK			0x00000F00		//认证
#define TIL_SS_SHAKEHANDLE_TRANSMIT		0x00000100		//

#define TIL_SS_RELEASE_MASK				0x0000F000		//连接的状态
#define TIL_SS_TORELEASE				0x00001000		//删除

#define ADDRESS_MAX_LENGTH		64
//////////////////////////////////////////////////////////////////////////////
#pragma warning (push)
#pragma warning (disable: 4251)
#pragma warning (disable: 4275)

class CNetThread;
class CBasicSessionNetClient;
class _BASIC_DLL_API CBasicSessionNet : public basiclib::CBasicObject, public basiclib::EnableRefPtr<CBasicSessionNet>
{
public:
	//global
	static void Initialize(pGetConfFunc func);
	static void CloseSocket();
public:
	typedef basiclib::CBasicRefPtr<CBasicSessionNet> CRefBasicSessionNet;
	typedef void(*pCallSameRefNetSessionFunc)(CBasicSessionNet* pRefNetSession, Net_PtrInt lRevert);

	typedef fastdelegate::FastDelegate4<CBasicSessionNetClient*, Net_UInt, Net_Int, const char *, Net_Int> HandleReceive;
	typedef fastdelegate::FastDelegate3<CBasicSessionNetClient*, Net_UInt, Net_Int, Net_Int> HandleSend;			//发送数据的通知
	typedef fastdelegate::FastDelegate2<CBasicSessionNetClient*, Net_UInt, Net_Int> HandleConnect;				//处理连接
	typedef fastdelegate::FastDelegate2<CBasicSessionNetClient*, Net_UInt, Net_Int> HandleDisConnect;			//断开的消息
	typedef fastdelegate::FastDelegate2<CBasicSessionNetClient*, Net_UInt, Net_Int> HandleIdle;				//空闲消息
	typedef fastdelegate::FastDelegate3<CBasicSessionNetClient*, Net_UInt, Net_Int, Net_Int> HandleError;			//错误消息

	void bind_rece(const HandleReceive& func){ m_funcReceive = func; }
	void bind_send(const HandleSend& func){ m_funcSend = func; }
	void bind_connect(const HandleConnect& func){ m_funcConnect = func; }
	void bind_disconnect(const HandleDisConnect& func){ m_funcDisconnect = func; }
	void bind_idle(const HandleIdle& func){ m_funcIdle = func; }
	void bind_error(const HandleError& func){ m_funcError = func; }
	
public:
	//1s
	virtual void OnTimer(Net_UInt nTick) = 0;
	//release the object
	virtual void Release();
	//close net
	void Close(BOOL bRemote = FALSE);
	//register presend filter
	int RegistePreSend(CBasicPreSend* pFilter, Net_UInt dwRegOptions = 0);
	
	const char* GetLibeventMethod();

	Net_UInt GetSessionStatus(Net_UInt dwMask){ return m_unSessionStatus & dwMask; }
	evutil_socket_t GetSocketFD(){ return m_socketfd; }
protected:
	CBasicSessionNet(Net_UInt nSessionID, bool bAddOnTimer = true);
	virtual ~CBasicSessionNet();

	virtual void InitMember();

	BOOL IsToClose(){ return GetSessionStatus(TIL_SS_CLOSE) >= TIL_SS_TOCLOSE; }
	BOOL IsRelease(){ return GetSessionStatus(TIL_SS_RELEASE_MASK) == TIL_SS_TORELEASE; }
	void SetToClose() { SetSessionStatus(TIL_SS_TOCLOSE, TIL_SS_CLOSE); }
	void SetToRelease(){ SetSessionStatus(TIL_SS_TORELEASE, TIL_SS_TORELEASE); }
	virtual BOOL CanClose();
	
	void SetSessionStatus(Net_UInt dwValue, Net_UInt dwMask){ m_unSessionStatus &= ~dwMask; m_unSessionStatus |= (dwValue & dwMask); }
protected:
	void SetLibEvent(pCallSameRefNetSessionFunc pCallback, Net_PtrInt lRevert = 0);
	virtual void ReleaseCallback();
	virtual void CloseCallback(BOOL bRemote, DWORD dwNetCode = 0);
protected:
	bool					m_bAddOnTimer;
	CRefBasicSessionNet		m_refSelf;
	HandleReceive			m_funcReceive;
	HandleSend				m_funcSend;
	HandleConnect			m_funcConnect;
	HandleDisConnect		m_funcDisconnect;
	HandleIdle				m_funcIdle;
	HandleError				m_funcError;

	CNetThread*				m_pThread;
	CBasicPreSend*			m_pPreSend;

	Net_UInt				m_unSessionStatus;				//状态  TIL_SS_*

	evutil_socket_t			m_socketfd;
	event					m_revent;

	friend class CNetThread;
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_BUFFER				4096
struct SendBuffer
{
	char	buf[MAX_BUFFER];
	int		len;

	SendBuffer()
	{
		memset(this, 0, sizeof(SendBuffer));
	}
};

typedef void(*pCallThreadSafeBeforeSendData)(SendDataToSendThread*);
class _BASIC_DLL_API CBasicSessionNetClient : public CBasicSessionNet
{
public:
	static CBasicSessionNetClient* CreateClient(Net_UInt nSessionID, bool bAddOnTimer = true){ return new CBasicSessionNetClient(nSessionID, bAddOnTimer); }
protected:
	CBasicSessionNetClient(Net_UInt nSessionID, bool bAddOnTimer);//must be new object
	virtual ~CBasicSessionNetClient();
public:
	Net_UInt GetSessionID(){ return m_nSessionID; }//uni id

	virtual Net_Int Connect(const char* lpszAddress);//formats [IPv6Address]:port || IPv4Address:port
	Net_Int DoConnect();
	BOOL IsConnected() { return GetSessionStatus(TIL_SS_LINK) == TIL_SS_CONNECTED; }
	BOOL IsTransmit(){ return GetSessionStatus(TIL_SS_SHAKEHANDLE_MASK) == TIL_SS_SHAKEHANDLE_TRANSMIT; }
	virtual Net_Int Send(void *pData, Net_Int cbData, Net_UInt dwFlag = 0);
	Net_Int Send(basiclib::CBasicSmartBuffer& smBuf, Net_UInt dwFlag = 0);
	void GetNetAddress(basiclib::CBasicString& strAddr){ strAddr = m_szPeerAddr; }
	UINT GetNetAddressPort(){ return m_nPeerPort; }
	virtual void OnTimer(Net_UInt nTickTime);
	BOOL IsRecTimeout(time_t tmNow, Net_UShort nTimeoutSecond);
	void GetReceiveTime(char* pBuffer, int nLength);//! 获取最后收到数据的时间
	virtual void GetNetStatus(CBasicString& strStatus);//! 获取状态信息
	void GetNetStatInfo(BasicNetStat& netState){ netState = m_stNet; }
protected:
	virtual Net_Int OnConnect(Net_UInt dwNetCode);
	virtual Net_Int OnDisconnect(Net_UInt dwNetCode);
	virtual void OnSendData(Net_UInt dwIoSize);
	virtual Net_Int OnReceive(Net_UInt dwNetCode, const char *pszData, Net_Int cbData);
	virtual Net_Int OnIdle(Net_UInt dwIdleCount);		//空闲处理
	Net_Int _handle_connect(Net_UInt dwNetCode);
	Net_Int _handle_disconnect(Net_UInt dwNetCode);
	Net_Int _handle_idle(Net_UInt dwNetCode);
	Net_Int _handle_error(Net_UInt dwNetCode, Net_Int lRetCode);
	Net_Int _handle_rece(Net_UInt dwNetCode, const char *pszData, Net_Int cbData);
	Net_Int _handle_send(Net_UInt dwNetCode, Net_Int cbSend);
protected:
	void InitClientEvent(evutil_socket_t socketfd, bool bAddWrite = true, bool bAddRead = true);
	void AddWriteEvent();
	virtual void InitMember();
	virtual void CloseCallback(BOOL bRemote, DWORD dwNetCode = 0);

	void Accept(evutil_socket_t s, sockaddr_storage& addr);
	friend void AcceptToSelf(CBasicSessionNetClient* p, evutil_socket_t s, sockaddr_storage& addr);

	void SetIdleCallback();
	void _OnIdle();

	Net_Int SendData(SendDataToSendThread& sendData, Net_UInt dwFlag);
	
	void OnReadEvent();
	void OnWriteEvent();
	friend void OnLinkRead(int fd, short event, void *arg);
	friend void OnLinkWrite(int fd, short event, void *arg);
	void OnReceiveData(const char* pszData, Net_UInt dwIoSize);
protected:
	BOOL IsSendBusy();					//判断是否正在发送
	void SetSendBusy(BOOL bBusy);		//设置是否在发送
	BOOL CheckSendBusy();				//检查是否在发送状态，如果不是，设置为发送状态，返回TRUE。否则返回FALSE。
protected:
	Net_Int PreReceiveData(Net_UInt dwNetCode, const char *pszData, Net_Int cbData);//过滤器接受数据
	void ResetPreSend();//\brief 重置过滤器状态
protected:
	Net_UInt				m_nSessionID;
	unsigned short			m_usTimeoutShakeHandle;		//default shakehandle timeout time
	Net_UInt				m_unIdleCount;				//进入空闲的次数
	Net_UShort				m_usRecTimeout;				//超时时间，0代表不超时
	event					m_wevent;
	char					m_szPeerAddr[ADDRESS_MAX_LENGTH];
	UINT					m_nPeerPort;
	basiclib::CBasicString	m_strConnectAddr;
	BasicNetStat			m_stNet;
	BasicNetStat			m_lastNet;
	//真正加入发送队列之前回调，比如加入序号包
	pCallThreadSafeBeforeSendData	m_threadSafeSendData;
	CBasicBitstream					m_bufCacheTmp;
private:
	//只在libevent线程使用
	void LibEventThreadSendData();
	void SendDataFromQueue();
	BOOL ReadBuffer(Net_Int lSend);
	virtual BOOL CanClose();
private:
	CMsgSendBufferQueue		m_msgQueue;
	SendBuffer				m_outBuffer;		//发送的缓存
};
typedef basiclib::CBasicRefPtr<CBasicSessionNetClient> CRefBasicSessionNetClient;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef basiclib::basic_map<Net_UInt, CRefBasicSessionNetClient>			MapClientSession;
typedef basiclib::basic_vector<CRefBasicSessionNetClient>					VTClientSession;
class _BASIC_DLL_API CBasicSessionNetServer : public CBasicSessionNet
{
public:
	static CBasicSessionNetServer* CreateServer(Net_UInt nSessionID){ return new CBasicSessionNetServer(nSessionID); }
protected:
	//must be new object
	CBasicSessionNetServer(Net_UInt nSessionID = 0);
	virtual ~CBasicSessionNetServer();

public:
	BOOL IsListen();
	/*formats
	-[IPv6Address]:port
	-IPv4Address:port
	*/
	virtual Net_Int Listen(const char* lpszAddress, bool bWaitSuccess = false);

	void SetClientRecTimeout(Net_UShort uTimesSecond){
		m_usRecTimeout = uTimesSecond;
	}
	//
	void SendToAll(void * pData, int nLength, DWORD dwFlag, bool bTrasmit = true);
	//断开所有连接,断开前的回调
	void CloseAllSession(const std::function<void(CBasicSessionNetClient* pSession)>& func);

	virtual void OnTimer(Net_UInt nTick);
	virtual void OnTimerWithAllClient(Net_UInt nTick, VTClientSession& vtClients);
	//! 获取状态信息
	virtual void GetNetStatus(CBasicString& strStatus);
public:
	//获取用户在线数
	long GetOnlineSessionCount()
	{
		return m_mapClientSession.size();
	}
	CRefBasicSessionNetClient GetClientBySessionID(Net_UInt nSessionID);
protected:
	void InitListenEvent(evutil_socket_t socketfd);
	void OnListenReadEvent();
	friend void OnLinkListenRead(int fd, short event, void *arg);
	void AcceptClient();
	void CopyClientSession(VTClientSession& vtClient);
	Net_Int OnClientDisconnectCallback(CBasicSessionNetClient* pClient, Net_UInt p2);//clientdisconnectcallback
	virtual void ReleaseCallback();
protected:
	virtual CBasicSessionNetClient* CreateServerClientSession(Net_UInt nSessionID);
	virtual CBasicSessionNetClient* ConstructSession(Net_UInt nSessionID);
protected:
	Net_UInt							m_nOnTimerTick;
	
	CMutex								m_mtxCSession;
	MapClientSession					m_mapClientSession;

	basiclib::CBasicString				m_strListenAddr;

	Net_UShort							m_usRecTimeout;				//超时时间，0代表不超时
/////////////////////////////////////////////////////////////////////////////////
//libevent线程下操作
private:
	Net_UInt GetNewSessionID();
private:
	Net_UInt								m_sessionIDMgr;
	CLockFreeMessageQueue<Net_UInt>			m_sessionIDQueue;
};
#pragma warning (pop)
__NS_BASIC_END
//////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack()
#endif 
