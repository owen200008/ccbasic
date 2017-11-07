/***********************************************************************************************
// 文件名:     net_mgr.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2016-9-12 11:50:18
// 内容描述:   定义TCP通信的基本类
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_NET_MGR_H
#define BASIC_NET_MGR_H

#include "net_socket.h"
#include "net.h"

__NS_BASIC_START
///////////////////////////////////////////////////////////////////////////////
struct CEventQueueItem{
	CBasicNet_Socket*								m_pRefNetSession;
	CBasicNet_Socket::pCallSameRefNetSessionFunc	m_pCallFunc;
	intptr_t										m_lRevert;
};

class CNetThread : public basiclib::CBasicObject{
public:
	CNetThread();
	virtual ~CNetThread();

	//! 发送事件
	void SetEvent(CBasicNet_Socket* pSession, CBasicNet_Socket::pCallSameRefNetSessionFunc pCallFunc, intptr_t lRevert);

	//! 准备关闭
	void ReadyToClose();

	//! 加入到全局消息队列
	void AddMessageQueue(CBasicNet_Socket* pSocket);

	//! 处理消息队列
	void RunMessageQueue();
#ifdef BASICWINDOWS_USE_IOCP
	//! IOCP线程
	static unsigned __stdcall ThreadIOCPFunc(void* lpWorkContext);

	//! 开始接收数据
	bool StartRecvData(CBasicNet_SocketTransfer* pSocket);

	//! 获取扩展func
	static IOCPExt_Func& GetExtFunc(){ return m_funcExt; }
#else
	//! 异步dns解析
	bool DNSParse(const char* pName, evdns_getaddrinfo_cb pCallback, CBasicNet_Socket* pSession);
#endif

#ifdef BASICWINDOWS_USE_IOCP
protected:
	static IOCPExt_Func			m_funcExt;
	static bool					m_bExtInit;
public:
	DWORD						m_dwThreadID;
	HANDLE						m_hCompletionPort;
	OVERLAPPEDPLUS				m_olFunc;
	DWORD						m_dwIoSize;
	ULONG						m_ulFlags;
#else
public:
	DWORD						m_dwThreadID;
	evutil_socket_t				m_pair[2];
	struct event_base*			m_base;
	struct evdns_base*			m_dnsbase;
	struct event				notify_event;
#endif

	//全局的消息通知
	basiclib::SpinLock				m_lockMsg;
	basiclib::CBasicSmartBuffer		m_smBuf;
	basiclib::CBasicSmartBuffer		m_smRunBuf;
};
/////////////////////////////////////////////////////////////////////////////
//net mgr
class CBasicNetMgv : public CBasicObject {
public:
	typedef basic_vector<CBasicNet_Socket*>						VTOnTimerSessionList;
	typedef VTOnTimerSessionList::iterator						VTOnTimerSessionListIterator;
public:
	CBasicNetMgv();
	virtual ~CBasicNetMgv();

	//! ontimer线程
	static THREAD_RETURN ThreadCheckFunc(void* lpWorkContext);

	//! 初始化线程
	void Initialize(pGetConfFunc func);

	//! 退出
	void CloseNetSocket();

	//! 加入timer
	void AddToTimer(CBasicNet_Socket* pSocket);

	//! 删除timer
	void DelToTimer(CBasicNet_Socket* pSocket);

	//! ontimer线程
	void OnTimer();
public:
	BOOL					m_bTimeToKill;
	bool					m_bTimerStop;

	VTOnTimerSessionList	m_vtOnTimerList;

	SpinLock				m_spinLockAdd;
	VTOnTimerSessionList	m_vtAddList;
	VTOnTimerSessionList	m_vtAddListDeal;

	VTOnTimerSessionList	m_vtDelList;
	VTOnTimerSessionList	m_vtDelListDeal;

	typedef basic_vector<CBasicSessionNet::CRefBasicSessionNet>	VTDeathSessionList;
	VTDeathSessionList		m_vtDeathSession;
	VTDeathSessionList		m_vtDeathSessionDeal;
};

//定义单态
typedef CBasicSingleton<CBasicNetMgv>	CBasicSingletonNetMgv;

__NS_BASIC_END
//////////////////////////////////////////////////////////////////////////////////////////////////

#endif