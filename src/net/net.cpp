#include "../inc/basic.h"

__NS_BASIC_START

#ifdef __BASICWINDOWS
#pragma comment(lib, "ws2_32.lib")
#elif defined(__LINUX)
#include <signal.h>
#endif

basiclib::CBasicString DefaultParamFuc(const char* pParam)
{
	return "";
}
static pGetConfFunc g_GetParamFunc = DefaultParamFuc;
void SetNetInitializeGetParamFunc(pGetConfFunc func)
{
	g_GetParamFunc = func;
}

///////////////////////////////////////////////////////////////////////////////
#define REG_EVENT_READ	0x0001
#define REG_EVENT_WRITE	0x0002
#define REG_EVENT_CLOSE	0x0004

#define EVF_READ	0x000001
#define EVF_CONNECT	0x000002
#define EVF_TIMER	0x000004
#define EVF_ADD		0x000100
#define EVF_DEL		0x000200
#define EVF_INIT	0x004000
#define EVF_REMOTE  0x010000

#pragma	pack(1)
struct CEventQueueItem
{
	CBasicSessionNet*								m_pRefNetSession;
	CBasicSessionNet::pCallSameRefNetSessionFunc	m_pCallFunc;
	Net_PtrInt										m_lRevert;
};
#pragma	pack()
///////////////////////////////////////////////////////////////////////////////////////////////////
typedef void(*pCallFuncForThread)(Net_PtrInt lRevert);

static BOOL			g_bTimeToKill = TRUE;

class CNetThread;
static CNetThread*	g_pEventThreads = nullptr;
static int			g_nEventThreadCount = 1;
static int			g_nLastThreadIndex = 0;
static int			g_nIntThreadCount = 0;
class CNetThread : public CBasicObject
{
public:
	CNetThread()
	{
		m_dwThreadID = 0;
		m_base = nullptr;
	}
	virtual ~CNetThread()
	{
	}
	void SetEvent(CBasicSessionNet* pSession, CBasicSessionNet::pCallSameRefNetSessionFunc pCallFunc, Net_PtrInt lRevert)
	{
		if (IsEventThread())
		{
			pSession->AddRef();
			SetSameThreadEvent(pSession, pCallFunc, lRevert);
			pSession->DelRef();
			return;
		}
		CEventQueueItem eventQueue;
		eventQueue.m_pRefNetSession = pSession;
		//increate no release
		eventQueue.m_pRefNetSession->AddRef();
		eventQueue.m_pCallFunc = pCallFunc;
		eventQueue.m_lRevert = lRevert;

		send(m_pair[1], (const char*)&eventQueue, sizeof(CEventQueueItem), 0);
	}

	bool IsEventThread()
	{
		return basiclib::BasicGetCurrentThreadId() == m_dwThreadID;
	}
	void SetSameThreadEvent(CBasicSessionNet* pSession, CBasicSessionNet::pCallSameRefNetSessionFunc pCallFunc, Net_PtrInt lRevert)
	{
		pCallFunc(pSession, lRevert);
	}

public:
	DWORD				m_dwThreadID;
	evutil_socket_t		m_pair[2];
	struct event_base*	m_base;
	struct event		notify_event;
};

/////////////////////////////////////////////////////////////////////////////
//net mgr
class CBasicNetMgv;
CBasicNetMgv* m_gNetMgrPoint = nullptr;
class CBasicNetMgv : public CBasicObject
{
public:
	CBasicNetMgv()
	{
		m_bCloseTimer = false;
		m_gNetMgrPoint = this;
		//create net
		CBasicSessionNet::Initialize(g_GetParamFunc);
	}
	virtual ~CBasicNetMgv()
	{
		//release net
		CBasicSessionNet::CloseSocket();
	}
public:
	event	m_clocktimer;
	bool	m_bCloseTimer;

	//operator in thread 0
	typedef basiclib::basic_vector<CBasicSessionNet*>::type	VTOnTimerSessionList;
	typedef VTOnTimerSessionList::iterator					VTOnTimerSessionListIterator;
	
	VTOnTimerSessionList	m_vtOnTimerList;
	VTOnTimerSessionList	m_vtAddList;
	VTOnTimerSessionList	m_vtDelList;

	typedef basiclib::basic_vector<CBasicSessionNet::CRefBasicSessionNet>::type	VTDeathSessionList;
	VTDeathSessionList		m_vtDeathSession;
};

//定义单态
typedef CBasicSingleton<CBasicNetMgv>	CBasicSingletonNetMgv;
////////////////////////////////////////////////////////////////////////

void ReadSelfOrder(int fd, short event, void *arg)
{
	CNetThread *thr = (CNetThread*)arg;

	CEventQueueItem eventQueue;
	int nReadLength = 0;
	do
	{
		nReadLength = recv(thr->m_pair[0], (char*)&eventQueue, sizeof(CEventQueueItem), 0);
		if (nReadLength == sizeof(CEventQueueItem))
		{
			//can't delete, the session must be free after delref()
			CBasicSessionNet::CRefBasicSessionNet pSession(eventQueue.m_pRefNetSession);
			//sign can delete ref
			eventQueue.m_pRefNetSession->DelRef();

			thr->SetSameThreadEvent(eventQueue.m_pRefNetSession, eventQueue.m_pCallFunc, eventQueue.m_lRevert);
		}
		else if (nReadLength < 0){
			break;
		}
	}
	while (nReadLength != sizeof(CEventQueueItem));
}

THREAD_RETURN WorkerThread(void *arg)
{
	srand(time(NULL) + basiclib::BasicGetTickTime() + basiclib::BasicGetCurrentThreadId());
	CNetThread *thr = (CNetThread*)arg;
	basiclib::BasicInterlockedIncrement((LONG*)&g_nIntThreadCount);
	event_base_loop(thr->m_base, 0);
	basiclib::BasicInterlockedDecrement((LONG*)&g_nIntThreadCount);
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
static void LogLibeventDNS(int is_warn, const char *msg) 
{
	basiclib::BasicLogEventV("%s: %s", is_warn ? "WARN" : "INFO", msg);
}

void NetOnTimer(evutil_socket_t fd, short event, void *arg)
{
	if (g_bTimeToKill)
	{
		//delete timer
		evtimer_del(&m_gNetMgrPoint->m_clocktimer);
		m_gNetMgrPoint->m_bCloseTimer = true;
		TRACE("delete net clocktimer!");
		return;
	}
	for (auto&session : m_gNetMgrPoint->m_vtAddList){
		m_gNetMgrPoint->m_vtOnTimerList.push_back(session);
	}
	m_gNetMgrPoint->m_vtAddList.clear();
	for (auto&session : m_gNetMgrPoint->m_vtDelList){
		CBasicNetMgv::VTOnTimerSessionList::iterator iter = find(m_gNetMgrPoint->m_vtOnTimerList.begin(), m_gNetMgrPoint->m_vtOnTimerList.end(), session);
		if (iter != m_gNetMgrPoint->m_vtOnTimerList.end())
		{
			m_gNetMgrPoint->m_vtOnTimerList.erase(iter);
		}
	}
	m_gNetMgrPoint->m_vtDelList.clear();
	for (auto&session : m_gNetMgrPoint->m_vtOnTimerList){
		session->OnTimer(0);
	}
	//延迟删除
	m_gNetMgrPoint->m_vtDeathSession.clear();

	return;
}

void CBasicSessionNet::Initialize(pGetConfFunc func)
{
	g_bTimeToKill = FALSE;
#ifdef __LINUX
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, 0);
#elif defined(__MAC)
	signal(SIGPIPE, SIG_IGN);
#endif

	struct event_config *cfg = event_config_new();
#ifdef __BASICWINDOWS
	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD(2, 2);

	(void)WSAStartup(wVersionRequested, &wsaData);

	//no support iocp now
	//event_config_set_flag(cfg, EVENT_BASE_FLAG_STARTUP_IOCP);
	//evthread_use_windows_threads();
#endif
	evdns_set_log_fn(LogLibeventDNS);

	g_nEventThreadCount = atol(func("ThreadCount").c_str());
	if (g_nEventThreadCount <= 0)
	{
		//default one thread
		g_nEventThreadCount = 1;
	}
	g_pEventThreads = new CNetThread[g_nEventThreadCount];

	//windows no multithread
	for (int i = 0; i < g_nEventThreadCount; i++)
	{
		CNetThread* pThread = &g_pEventThreads[i];
		
		if (evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, pThread->m_pair) == -1)
		{
			basiclib::BasicLogEventError("create evutil_socketpair error");
			exit(1);
		}
		//create self eventbase
		pThread->m_base = event_base_new_with_config(cfg);
		event_set(&pThread->notify_event, pThread->m_pair[0], EV_READ | EV_PERSIST, ReadSelfOrder, pThread);
		event_base_set(pThread->m_base, &pThread->notify_event);
		if (event_add(&pThread->notify_event, NULL) == -1)
		{
			basiclib::BasicLogEventError("libevent eventadd error");
			exit(1);
		}
		//create timer for 0 thread, all session shedule
		if (i == 0)
		{
			event_set(&m_gNetMgrPoint->m_clocktimer, -1, EV_PERSIST, NetOnTimer, 0);
			event_base_set(pThread->m_base, &m_gNetMgrPoint->m_clocktimer);

			timeval tv;
			tv.tv_sec = 1;
			tv.tv_usec = 0;
			evtimer_add(&m_gNetMgrPoint->m_clocktimer, &tv);
		}
		
		BasicCreateThread(WorkerThread, pThread, &(pThread->m_dwThreadID));
	}
	
	//init all thread
	while (g_nIntThreadCount != g_nEventThreadCount)
	{
		basiclib::BasicSleep(100);
	}
	event_config_free(cfg);
}

void CBasicSessionNet::CloseSocket()
{
	TRACE("Start CloseSocket!");
	g_bTimeToKill = TRUE;
	if (g_nEventThreadCount > 0 && g_pEventThreads != nullptr)
	{
		int nTimes = 0;
		while (!m_gNetMgrPoint->m_bCloseTimer)
		{
			nTimes++;
			basiclib::BasicSleep(100);
			if (nTimes > 100)
			{
				break;
			}
		}

		for (int i = 0; i < g_nEventThreadCount; i++)
		{
			CNetThread* pThread = &g_pEventThreads[i];
			event_base_loopbreak(pThread->m_base);
			//event_base_dispatch(pThread->m_base);
		}
#ifdef __BASICWINDOWS
		WSACleanup();
#endif
		nTimes = 0;
		while (g_nIntThreadCount != 0)
		{
			nTimes++;
			basiclib::BasicSleep(100);
			if (nTimes > 100)
			{
				break;
			}
		}

		for (int i = 0; i < g_nEventThreadCount; i++)
		{
			CNetThread* pThread = &g_pEventThreads[i];
			event_base_free(pThread->m_base);
		}
		delete[] g_pEventThreads;
		g_pEventThreads = nullptr;
	}

	TRACE("End CloseSocket!");
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CBasicSessionNet::CBasicSessionNet(Net_UInt nSessionID, bool bAddOnTimer)
{
	//init net
	CBasicSingletonNetMgv::Instance();
	m_refSelf = this;
	m_pThread = &g_pEventThreads[nSessionID % g_nEventThreadCount];
	m_pPreSend = nullptr;
	m_bAddOnTimer = bAddOnTimer;
	m_socketfd = -1;
	m_unSessionStatus = 0;
	if (m_bAddOnTimer)
	{
		g_pEventThreads[0].SetEvent(this, [](CBasicSessionNet* pSession, Net_PtrInt lRevert)->void{
			m_gNetMgrPoint->m_vtAddList.push_back(pSession);
		}, 0);
	}
}

CBasicSessionNet::~CBasicSessionNet()
{
	ASSERT(m_refSelf == nullptr || m_refSelf->GetRef() == 0);
	if (m_pPreSend != NULL)
	{
		delete m_pPreSend;
	}
}

void CBasicSessionNet::InitMember()
{
	m_socketfd = -1;
	if (IsRelease())
	{
		Release();
	}
	else
	{
		m_unSessionStatus = 0;
	}	
}

void CBasicSessionNet::Release()
{
	if (m_refSelf == nullptr)
		return;
	SetToRelease();
	Close(FALSE);
	SetLibEvent([](CBasicSessionNet* pSession, Net_PtrInt lRevert)->void{
		if (pSession->GetSocketFD() == -1)
			pSession->ReleaseCallback();
	}, 0);	
}

void CBasicSessionNet::ReleaseCallback()
{
	g_pEventThreads[0].SetEvent(this, [](CBasicSessionNet* pSession, Net_PtrInt lRevert)->void{
		bool bAddOnTimer = lRevert;
		m_gNetMgrPoint->m_vtDeathSession.push_back(pSession);
		if (bAddOnTimer)
			m_gNetMgrPoint->m_vtDelList.push_back(pSession);
	}, m_bAddOnTimer);
	m_bAddOnTimer = false;

	if (m_refSelf == nullptr)
		return;
	m_refSelf = nullptr;
}

BOOL CBasicSessionNet::CanClose()
{
	if (GetSessionStatus(TIL_SS_LINK) == TIL_SS_CONNECTING)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CBasicSessionNetClient::CanClose()
{
	BOOL bRet = CBasicSessionNet::CanClose();
	if (bRet)
	{
		if (m_outBuffer.len <= 0 && m_msgQueue.GetMQLength() == 0)
			return TRUE;
	}
	return !IsConnected();
}

void CBasicSessionNet::Close(BOOL bRemote)
{
	if (m_socketfd == -1)
		return;

	SetToClose();
	//may be loop, take care
	SetLibEvent([](CBasicSessionNet* pSession, Net_PtrInt lRevert)->void{
		if (pSession->CanClose())
		{
			BOOL bRemote = lRevert;
			pSession->CloseCallback(bRemote);
		}
	}, bRemote);
}

int CBasicSessionNet::RegistePreSend(CBasicPreSend* pFilter, Net_UInt dwRegOptions)
{
	if (pFilter != NULL)
	{
		if (m_pPreSend != NULL)
		{
			delete m_pPreSend;
			m_pPreSend = NULL;
		}
		if (m_pPreSend == NULL)
		{
			m_pPreSend = pFilter->Construct();
		}
	}
	return 1;
}

void CBasicSessionNet::SetLibEvent(pCallSameRefNetSessionFunc pCallback, Net_PtrInt lRevert)
{
	m_pThread->SetEvent(this, pCallback, lRevert);
}

void CBasicSessionNet::CloseCallback(BOOL bRemote, DWORD dwNetCode)
{
	if (m_socketfd != -1)
	{
		event_del(&m_revent);
		evutil_closesocket(m_socketfd);
		InitMember();
	}
}

//slow sleep
const char* CBasicSessionNet::GetLibeventMethod()
{
	return event_base_get_method(m_pThread->m_base);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AcceptToSelf(CBasicSessionNetClient* p, evutil_socket_t s, sockaddr_storage& addr)
{
	p->Accept(s, addr);
}

void OnLinkListenRead(int fd, short event, void *arg)
{
	CBasicSessionNetServer *pLink = (CBasicSessionNetServer *)arg;
	if (pLink && pLink->m_socketfd == fd)
	{
		pLink->OnListenReadEvent();
	}
}

CBasicSessionNetServer::CBasicSessionNetServer(Net_UInt nSessionID) : CBasicSessionNet(nSessionID)
{
	m_nOnTimerTick = 0;
	m_sessionIDMgr = nSessionID;
	m_sessionIDQueue.SetOverLoadLength(0x0FFFFFFF);
	m_usRecTimeout = 0;
}

CBasicSessionNetServer::~CBasicSessionNetServer()
{

}

Net_UInt CBasicSessionNetServer::GetNewSessionID()
{
	Net_UInt nSessionID = 0;
	if (m_sessionIDQueue.MQPop(&nSessionID) == 0)
		return nSessionID;
	return basiclib::BasicInterlockedIncrement((LONG*)&m_sessionIDMgr);
}

BOOL CBasicSessionNetServer::IsListen()
{
	return (m_socketfd != -1);
}

Net_Int CBasicSessionNetServer::Listen(const char* lpszAddress, bool bWaitSuccess)
{
	if (IsListen())
		return BASIC_NET_ALREADY_LISTEN;
	if (IsToClose())
		return BASIC_NET_TOCLOSE_ERROR;

	m_strListenAddr = lpszAddress;

	Net_Int lReturn = BASIC_NET_OK;
	evutil_socket_t socketfd = -1;
	do
	{
		sockaddr_storage addr;
		int addrlen = sizeof(addr);
		if(evutil_parse_sockaddr_port(lpszAddress, (sockaddr*)&addr, &addrlen) != 0)
		{
			lReturn = BASIC_NET_ADDRESS_ERROR;
			break;
		}
		socketfd = socket(addr.ss_family, SOCK_STREAM, 0);
		if (socketfd == -1)
		{
			lReturn = BASIC_NET_SOCKET_ERROR;
			break;
		}

		evutil_make_socket_nonblocking(socketfd);
		evutil_make_listen_socket_reuseable(socketfd);
		evutil_make_listen_socket_reuseable_port(socketfd);

		// bind our name to the socket
		int nRet = ::bind(socketfd, (::sockaddr*)&addr, addrlen);   //I know this

		if (nRet != 0)
		{
			lReturn = BASIC_NET_BIND_ERROR;
			break;
		}

		// Set the socket to listen
		nRet = listen(socketfd, 0x7fffffff);
		if (nRet != 0)
		{
			lReturn = BASIC_NET_LISTEN_ERROR;
			break;
		}
	} while (0);

	if (lReturn == BASIC_NET_OK)
	{
		SetLibEvent([](CBasicSessionNet* pSession, Net_PtrInt lRevert)->void{
			evutil_socket_t socketfd = lRevert;
			((CBasicSessionNetServer*)pSession)->InitListenEvent(lRevert);
		}, socketfd);
		if (bWaitSuccess)
		{
			while (!IsListen())
			{
				basiclib::BasicSleep(100);
			}
		}
	}
	else if (socketfd != -1)
	{
		evutil_closesocket(socketfd);
	}
	return lReturn;
}
void CBasicSessionNetServer::InitListenEvent(evutil_socket_t socketfd)
{
	m_socketfd = socketfd;
	event_set(&m_revent, m_socketfd, EV_READ | EV_PERSIST, OnLinkListenRead, this);
	event_base_set(m_pThread->m_base, &m_revent);
	event_add(&m_revent, NULL);
}

void CBasicSessionNetServer::OnListenReadEvent()
{
	if (IsListen())
	{
		AcceptClient();
	}
}

void CBasicSessionNetServer::AcceptClient()
{
	if (IsToClose())
	{
		return;
	}
	sockaddr_storage addr;
	memset(&addr, 0, sizeof(addr));
	socklen_t addrlen = sizeof(addr);
	evutil_socket_t s = accept(m_socketfd, (::sockaddr*)&addr, &addrlen);   //I know this
	if (s == -1)
	{
		return;
	}
	CBasicSessionNetClient* pAcceptSession = CreateServerClientSession(GetNewSessionID());
	AcceptToSelf(pAcceptSession, s, addr);

	basiclib::CSingleLock lock(&m_mtxCSession, TRUE);
	m_mapClientSession[pAcceptSession->GetSessionID()] = pAcceptSession;
}

Net_Int CBasicSessionNetServer::ClientDisconnectCallback(CBasicSessionNetClient* pClient, Net_UInt p2)
{
	//disconnect
	Net_Int lRet = m_funcDisconnect ? m_funcDisconnect(pClient, p2) : BASIC_NET_OK;

	//delete
	Net_UInt nSessionID = pClient->GetSessionID();
	basiclib::CSingleLock lock(&m_mtxCSession, TRUE);
	m_mapClientSession.erase(nSessionID);
	lock.Unlock();
	//release
	pClient->Release();

	//回收sessionid
	m_sessionIDQueue.MQPush(&nSessionID);

	return lRet;
}

CBasicSessionNetClient* CBasicSessionNetServer::CreateServerClientSession(Net_UInt nSessionID)
{
	CBasicSessionNetClient* pNotify = ConstructSession(nSessionID);
	pNotify->bind_rece(m_funcReceive);
	pNotify->bind_send(m_funcSend);
	pNotify->bind_connect(m_funcConnect);
	pNotify->bind_disconnect(MakeFastFunction(this, &CBasicSessionNetServer::ClientDisconnectCallback));
	pNotify->bind_idle(m_funcIdle);
	pNotify->bind_error(m_funcError);
	if (m_pPreSend != NULL)
	{
		pNotify->RegistePreSend(m_pPreSend);
	}
	return pNotify;
}

CBasicSessionNetClient* CBasicSessionNetServer::ConstructSession(Net_UInt nSessionID)
{
	return CBasicSessionNetClient::CreateClient(nSessionID, false);
}

//外部ontimer驱动1s
void CBasicSessionNetServer::OnTimer(Net_UInt nTick)
{
	if (IsRelease())
	{
		Release();
		return;
	}
	m_nOnTimerTick++;
	if (m_nOnTimerTick % 10 == 0)
	{
		if (!m_strListenAddr.IsEmpty() && !IsListen())
		{
			Net_Int lRet = Listen(m_strListenAddr.c_str());
			if (BASIC_NET_OK == lRet)
			{
				basiclib::BasicLogEventV("ListenPort [%s] OK", m_strListenAddr.c_str());
			}
			else if (BASIC_NET_ALREADY_LISTEN != lRet)
			{
				Close();
			}
		}
	}

	bool bCheckTimeOut = (m_nOnTimerTick % 10 == 9);
	time_t tmNow = time(NULL);
	VTClientSession vtClient;
	CopyClientSession(vtClient);
	//10s检查一次timeout
	if (m_nOnTimerTick % 10 == 9){
		for (auto& client : vtClient){
			client->OnTimer(m_nOnTimerTick);
			if (client->IsRecTimeout(tmNow, m_usRecTimeout)){
				client->Close(FALSE);
			}
		}
	}
	else{
		for (auto& client : vtClient){
			client->OnTimer(m_nOnTimerTick);
		}
	}

	OnTimerWithAllClient(m_nOnTimerTick, vtClient);
}
void CBasicSessionNetServer::OnTimerWithAllClient(Net_UInt nTick, VTClientSession& vtClients)
{
	
}

CRefBasicSessionNetClient CBasicSessionNetServer::GetClientBySessionID(Net_UInt nSessionID)
{
	basiclib::CSingleLock lock(&m_mtxCSession, TRUE);
	MapClientSession::iterator iter = m_mapClientSession.find(nSessionID);
	if (iter != m_mapClientSession.end())
	{
		return iter->second;
	}
	return nullptr;
}



void CBasicSessionNetServer::CopyClientSession(VTClientSession& vtClient)
{
	basiclib::CSingleLock lock(&m_mtxCSession, TRUE);
	int nSize = m_mapClientSession.size();
	vtClient.reserve(nSize);
	for (auto& client : m_mapClientSession)
	{
		vtClient.push_back(client.second);
	}
}

//断开所有连接,断开前的回调
void CBasicSessionNetServer::CloseAllSession(const std::function<void(CBasicSessionNetClient* pSession)>& func)
{
	VTClientSession vtUser;
	CopyClientSession(vtUser);

	for (auto& client : vtUser)
	{
		func(client.GetResFunc());
		client->Close();
	}
}
void CBasicSessionNetServer::SendToAll(void * pData, int nLength, DWORD dwFlag, bool bTrasmit)
{
	VTClientSession vtUser;
	CopyClientSession(vtUser);
	if (bTrasmit){
		for (auto& client : vtUser){
			if (client->IsConnected() && client->IsTransmit()){
				client->Send(pData, nLength, dwFlag);
			}
		}
	}
	else{
		for (auto& client : vtUser){
			if (client->IsConnected()){
				client->Send(pData, nLength, dwFlag);
			}
		}
	}
}
//! 获取状态信息
void CBasicSessionNetServer::GetNetStatus(CBasicString& strStatus)
{
	VTClientSession vtUser;
	CopyClientSession(vtUser);

	basiclib::CBasicString strVal;
	strVal.Format("监听: %s  连接队列如下：\r\n", m_strListenAddr.c_str());
	for (auto& client : vtUser)
	{
		client->GetNetStatus(strVal);
	}
	strStatus += strVal;
}

void CBasicSessionNetServer::ReleaseCallback()
{
	//如果还有用户
	if (GetOnlineSessionCount() > 0){
		CloseAllSession([](...)->void{
		});
		return;
	}
	CBasicSessionNet::ReleaseCallback();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void OnLinkRead(int fd, short event, void *arg)
{
	CRefBasicSessionNetClient pLink = (CBasicSessionNetClient*)arg;
	if (pLink != nullptr && pLink->m_socketfd == fd)
	{
		pLink->OnReadEvent();
	}
}

void OnLinkWrite(int fd, short event, void *arg)
{
	CRefBasicSessionNetClient pLink = (CBasicSessionNetClient*)arg;
	if (pLink != nullptr && pLink->m_socketfd == fd)
	{
		pLink->OnWriteEvent();
	}
}


CBasicSessionNetClient::CBasicSessionNetClient(Net_UInt nSessionID, bool bAddOnTimer) : CBasicSessionNet(nSessionID, bAddOnTimer)
{
	m_nSessionID = nSessionID;
	m_usTimeoutShakeHandle = 10;
	m_threadSafeSendData = nullptr;
	memset(m_szPeerAddr, 0, sizeof(m_szPeerAddr));
	m_nPeerPort = 0;
	m_unIdleCount = 0;
	m_usRecTimeout = 0;
}

CBasicSessionNetClient::~CBasicSessionNetClient()
{

}

void CBasicSessionNetClient::InitMember()
{
	CBasicSessionNet::InitMember();
	memset(m_szPeerAddr, 0, sizeof(m_szPeerAddr));
	m_nPeerPort = 0;
	m_unIdleCount = 0;
	
	m_outBuffer.len = 0;
	m_msgQueue.Drop_Queue([](SendDataToSendThread* pBuffer, void*)->void{
		pBuffer->ReleaseData();
	}, nullptr);
}

void CBasicSessionNetClient::CloseCallback(BOOL bRemote, DWORD dwNetCode)
{
	if (m_socketfd != -1)
	{
		if (bRemote)
		{
			dwNetCode |= BASIC_NETCODE_CLOSE_REMOTE;
		}
		event_del(&m_wevent);
		CBasicSessionNet::CloseCallback(bRemote);

		OnDisconnect(dwNetCode);
	}
}

void CBasicSessionNetClient::Accept(evutil_socket_t s, sockaddr_storage& addr)
{
	socklen_t addrlen = sizeof(addr);

#if defined(__MAC)
	int set = 1;
	setsockopt(s, SOL_SOCKET, SO_NOSIGPIPE, (void*)&set, sizeof(int));
#endif
	SetSessionStatus(TIL_SS_CONNECTING, TIL_SS_LINK);

	int nFamily = addr.ss_family;
	memset(&addr, 0, sizeof(addrlen));
	getpeername(s, (::sockaddr*)&addr, &addrlen);

	if (nFamily == AF_INET)
	{
		char szBuf[128] = { 0 };
		sockaddr_in* pSockAddr = (struct sockaddr_in*)&addr;
		m_nPeerPort = ntohs(pSockAddr->sin_port);		//I know this
		strncpy(m_szPeerAddr, evutil_inet_ntop(AF_INET, &pSockAddr->sin_addr, szBuf, sizeof(szBuf)), ADDRESS_MAX_LENGTH);		//I know this
	}
	else
	{
		char szBuf[128] = { 0 };
		sockaddr_in6* pSockAddr = (struct sockaddr_in6*)&addr;
		m_nPeerPort = ntohs(pSockAddr->sin6_port);		//I know this
		strncpy(m_szPeerAddr, evutil_inet_ntop(AF_INET6, &pSockAddr->sin6_addr, szBuf, sizeof(szBuf)), ADDRESS_MAX_LENGTH);		//I know this
	}
	
	evutil_make_socket_nonblocking(s);
	evutil_make_listen_socket_reuseable(s);
	evutil_make_listen_socket_reuseable_port(s);

	SetLibEvent([](CBasicSessionNet* pSession, Net_PtrInt lRevert)->void{
		evutil_socket_t socketfd = lRevert;
		((CBasicSessionNetClient*)pSession)->InitClientEvent(socketfd, false);
		((CBasicSessionNetClient*)pSession)->OnConnect(BASIC_NETCODE_SUCC);
	}, s);
}

void CBasicSessionNetClient::InitClientEvent(evutil_socket_t socketfd, bool bAddWrite)
{
	m_socketfd = socketfd;
	//read data
	event_set(&m_revent, m_socketfd, EV_READ | EV_PERSIST, OnLinkRead, this);
	event_base_set(m_pThread->m_base, &m_revent);
	event_add(&m_revent, NULL);

	//write data
	event_set(&m_wevent, m_socketfd, EV_WRITE, OnLinkWrite, this);
	event_base_set(m_pThread->m_base, &m_wevent);
	if (bAddWrite)
		event_add(&m_wevent, NULL);
}
void CBasicSessionNetClient::AddWriteEvent()
{
	event_add(&m_wevent, NULL);
}

Net_Int CBasicSessionNetClient::OnConnect(Net_UInt dwNetCode)
{
	if (dwNetCode & BASIC_NETCODE_SUCC)
	{
		//设置最后收到的时间
		m_stNet.OnReceiveData(0);
		SetSessionStatus(TIL_SS_CONNECTED, TIL_SS_LINK);
		m_stNet.Empty();
	}
	else
	{
		CloseCallback(TRUE, BASIC_NETCODE_CONNET_FAIL);
		return BASIC_NET_GENERIC_ERROR;
	}

	ResetPreSend();

	Net_Int lRet = _handle_connect(dwNetCode);
	//如果函数返回 BASIC_NET_HC_RET_HANDSHAKE，表示该连接需要进行握手。否则认为握手成功。
	if (lRet == BASIC_NET_OK)
	{
		SetSessionStatus(TIL_SS_SHAKEHANDLE_TRANSMIT, TIL_SS_SHAKEHANDLE_MASK);
	}
	else if (lRet == BASIC_NET_GENERIC_ERROR)
	{
		Close(FALSE);
	}
	return lRet;
}

//out timer
void CBasicSessionNetClient::OnTimer(Net_UInt nTick)
{
	if (GetSessionStatus(TIL_SS_CLOSE) == TIL_SS_TOCLOSE)
	{
		Close(FALSE);
	}
	else if (IsConnected())
	{
		//每次发送会重置idlecount
		_OnIdle();
		//10s检查超时
		if (nTick % 10 == 9){
			time_t tmNow = time(NULL);
			if (IsRecTimeout(tmNow, m_usRecTimeout)){
				Close(FALSE);
			}
		}
	}
}

BOOL CBasicSessionNetClient::IsRecTimeout(time_t tmNow, Net_UShort nTimeoutSecond)
{
	if (tmNow - m_stNet.m_tmLastRecTime >= nTimeoutSecond && nTimeoutSecond != 0)
		return TRUE;
	return FALSE;
}

void CBasicSessionNetClient::GetReceiveTime(char* pBuffer, int nLength)
{
	if (m_stNet.m_tmLastRecTime > 0)
	{
		basiclib::CTime tm(m_stNet.m_tmLastRecTime);
		tm.FormatToBuffer("%H:%M:%S", pBuffer, nLength);
	}
}
//! 获取状态信息
void CBasicSessionNetClient::GetNetStatus(CBasicString& strStatus)
{
	BasicNetStat stNet;
	GetNetStatInfo(stNet);
	Net_UInt dwLinkNetStatus = GetSessionStatus(TIL_SS_LINK);
	Net_UInt dwCloseNetStatus = GetSessionStatus(TIL_SS_CLOSE);
	char szBuf[32] = { 0 };
	if (dwCloseNetStatus != TIL_SS_NORMAL){
		switch (dwCloseNetStatus){
		case TIL_SS_TOCLOSE:{
				strcpy(szBuf, "TOCLOSE");
			}
			break;
		}
	}
	else{
		switch (dwLinkNetStatus){
		case TIL_SS_IDLE:
			strcpy(szBuf, "IDLE");
			break;
		case TIL_SS_CONNECTING:
			strcpy(szBuf, "CONNECTING");
			break;
		case TIL_SS_CONNECTED:{
				if (GetSessionStatus(TIL_SS_SHAKEHANDLE_MASK) != TIL_SS_SHAKEHANDLE_TRANSMIT)
					strcpy(szBuf, "CONNECTED");
				else
					strcpy(szBuf, "OK");
			}
			break;
		}
	}
	char szBufTime[32] = { 0 };
	GetReceiveTime(szBufTime, 32);

	double dSendRate = 0;
	double dRecvRate = 0;
	stNet.GetTransRate(m_lastNet, dSendRate, dRecvRate);
	CBasicString strTemp;
	strTemp.Format("Link: %s LastReceive: %s Timeout: %d \r\nStatus: %s R%uK S%uK R:%.2fKB/s S:%.2fKB/s\t\r\n", 
		m_strConnectAddr.IsEmpty() ? m_szPeerAddr : m_strConnectAddr.c_str(), szBufTime, m_usRecTimeout, szBuf, stNet.m_dwReceBytes / 1024, stNet.m_dwSendBytes / 1024, dRecvRate, dSendRate);

	strStatus += strTemp;
}

Net_Int CBasicSessionNetClient::OnDisconnect(Net_UInt dwNetCode)
{
	_handle_disconnect(dwNetCode);
	return BASIC_NET_OK;
}

void CBasicSessionNetClient::_OnIdle()
{
	m_unIdleCount++;
	if (GetSessionStatus(TIL_SS_SHAKEHANDLE_MASK) != TIL_SS_SHAKEHANDLE_TRANSMIT && m_unIdleCount > m_usTimeoutShakeHandle)
	{
		Close();
		return;
	}
	OnIdle(m_unIdleCount);
}

Net_Int CBasicSessionNetClient::DoConnect()
{
	Net_UInt dwRelease = GetSessionStatus(TIL_SS_RELEASE_MASK);
	Net_UInt dwLinkNetStatus = GetSessionStatus(TIL_SS_LINK);
	Net_UInt dwCloseNetStatus = GetSessionStatus(TIL_SS_CLOSE);
	if (dwRelease != 0)
	{
		return BASIC_NET_RELEASE_ERROR;
	}
	if (dwCloseNetStatus != TIL_SS_NORMAL)
	{
		Net_Int lRet = BASIC_NET_GENERIC_ERROR;
		switch (dwCloseNetStatus)
		{
		case TIL_SS_TOCLOSE:
		{
			lRet = BASIC_NET_TOCLOSE_ERROR;
			break;
		}
		}
		return lRet;
	}
	if (dwLinkNetStatus != TIL_SS_IDLE)
	{
		Net_Int lRet = BASIC_NET_GENERIC_ERROR;
		switch (dwLinkNetStatus)
		{
		case TIL_SS_CONNECTING:
		{
			lRet = BASIC_NET_CONNECTING_ERROR;
			break;
		}
		case TIL_SS_CONNECTED:
		{
			lRet = BASIC_NET_ALREADY_CONNECT;
			break;
		}
		}
		return lRet;
	}
	if (m_strConnectAddr.IsEmpty())
	{
		return BASIC_NET_INVALID_ADDRESS;
	}
	
	Net_Int lReturn = BASIC_NET_OK;
	evutil_socket_t socketfd = -1;
	do
	{
		sockaddr_storage addr;
		int addrlen = sizeof(addr);
		if (evutil_parse_sockaddr_port(m_strConnectAddr.c_str(), (sockaddr*)&addr, &addrlen) != 0)
		{
			lReturn = BASIC_NET_ADDRESS_ERROR;
			break;
		}
		socketfd = socket(addr.ss_family, SOCK_STREAM, 0);
		if (socketfd == -1)
		{
			lReturn = BASIC_NET_SOCKET_ERROR;
			break;
		}

		evutil_make_socket_nonblocking(socketfd);
		evutil_make_listen_socket_reuseable(socketfd);
		evutil_make_listen_socket_reuseable_port(socketfd);

		int nRet = connect(socketfd, (::sockaddr*)&addr, addrlen);
		if (nRet == 0)
		{
			SetLibEvent([](CBasicSessionNet* pSession, Net_PtrInt lRevert)->void{
				evutil_socket_t socketfd = lRevert;
				((CBasicSessionNetClient*)pSession)->InitClientEvent(socketfd, false);
				((CBasicSessionNetClient*)pSession)->OnConnect(BASIC_NETCODE_SUCC);
			}, socketfd);
		}
#ifdef __BASICWINDOWS
		else if (errno == EINPROGRESS || WSAGetLastError() == WSAEWOULDBLOCK)
#else
		else if (errno == EINPROGRESS)
#endif
		{
			SetSessionStatus(TIL_SS_CONNECTING, TIL_SS_LINK);
			//wait for write onconnect
			SetLibEvent([](CBasicSessionNet* pSession, Net_PtrInt lRevert)->void{
				evutil_socket_t socketfd = lRevert;
				((CBasicSessionNetClient*)pSession)->InitClientEvent(socketfd, true);
			}, socketfd);
		}
		else
		{
			//int nRetErrorNo = errno;
			lReturn = BASIC_NET_GENERIC_ERROR;
		}
		
	} while (0);

	if (lReturn != BASIC_NET_OK)
	{
		evutil_closesocket(socketfd);
	}
	return lReturn;
}

Net_Int CBasicSessionNetClient::Connect(const char* lpszAddress)
{
	if (lpszAddress == nullptr || lpszAddress[0] == '\0')
	{
		return BASIC_NET_INVALID_ADDRESS;
	}
	m_strConnectAddr = lpszAddress;

	return DoConnect();
}

Net_Int CBasicSessionNetClient::Send(void *pData, Net_Int cbData, Net_UInt dwFlag)
{
	if (m_pPreSend != NULL)
	{
		char* pSendData = (char*)pData;
		long nSendData = cbData;

		CBasicSmartBuffer bufTmp;
		int rRet = PACK_FILTER_SEARCH;
		rRet = m_pPreSend->OnPreSend(pSendData, nSendData, dwFlag, bufTmp);
		if (rRet == PACK_FILTER_SKIP)
		{
		}
		else if (rRet == PACK_FILTER_HANDLED || rRet == PACK_FILTER_SEARCH)
		{
			pSendData = bufTmp.GetDataBuffer(nSendData);
		}
		else
		{
			_handle_error(BASIC_NETCODE_FILTER_ERROR, BASIC_NET_FILTER_WRITE_FAIL);
			return BASIC_NET_FILTER_WRITE_FAIL;
		}
		return SendData(pSendData, nSendData, dwFlag);
	}
	else
	{
		return SendData(pData, cbData, dwFlag);
	}
}

BOOL CBasicSessionNetClient::ReadBuffer(Net_Int lSend)
{
	int nLeft = m_outBuffer.len - lSend;
	if (nLeft > 0)
	{
		if (lSend > 0)
		{
			memmove(m_outBuffer.buf, &m_outBuffer.buf[lSend], nLeft);
			m_outBuffer.len = nLeft;
		}
	}
	else
	{
		m_outBuffer.len = m_msgQueue.ReadBuffer(m_outBuffer.buf, MAX_BUFFER);
	}
	return m_outBuffer.len > 0;
}

//保证是libevent线程处理
void CBasicSessionNetClient::SendDataFromQueue()
{
	if (!IsConnected())
	{
		return;
	}
	BOOL bError = FALSE;
	int nTotalSend = 0;
	Net_Int lSend = 0;
	while (ReadBuffer(lSend))
	{
		lSend = send(m_socketfd, m_outBuffer.buf, m_outBuffer.len, 0);
		if (lSend >= 0)
		{
			nTotalSend += lSend;
		}
		else
		{
			int nNumber = errno;
			if (nNumber == EAGAIN)
			{
				AddWriteEvent();
				break;
			}
			else if (nNumber == EINTR)
			{
				lSend = 0;
				continue;
			}
			else
			{
#ifdef __BASICWINDOWS
				if (lSend == SOCKET_ERROR)
				{
					DWORD dwLastError = WSAGetLastError();   //I know this
					if (dwLastError == WSAEWOULDBLOCK)
					{
						AddWriteEvent();
					}
					else if (dwLastError != WSA_IO_PENDING)
					{
						bError = TRUE;
					}
				}
#else
				bError = TRUE;
#endif
				break;
			}
		}
	}
	if (bError)
	{
		//有错误清空缓存
		m_outBuffer.len = 0;
		m_msgQueue.Drop_Queue([](SendDataToSendThread* pBuffer, void*)->void{
			pBuffer->ReleaseData();
		}, nullptr);
		Close(TRUE);
	}
	else
	{
		OnSendData(nTotalSend);
	}
}

void CBasicSessionNetClient::LibEventThreadSendData()
{
	SendDataFromQueue();
}

Net_Int CBasicSessionNetClient::SendData(void *pData, Net_Int cbData, Net_UInt dwFlag)
{
	if (IsToClose())
	{
		return BASIC_NET_TOCLOSE_ERROR;
	}
	if (!IsConnected())
	{
		return BASIC_NET_NO_CONNECT;
	}
	if (cbData <= 0)
	{
		return 0;
	}
	SendDataToSendThread sendThreadData((char*)pData, cbData); 
	m_msgQueue.MQPush(&sendThreadData);

	SetLibEvent([](CBasicSessionNet* pSession, Net_PtrInt lRevert)->void{
		CBasicSessionNetClient* pClientSession = (CBasicSessionNetClient*)pSession;
		pClientSession->LibEventThreadSendData();
	});

	return cbData;
}

void CBasicSessionNetClient::OnSendData(Net_UInt dwIoSize)
{
	if (IsToClose() || !IsConnected())
		return;
	m_stNet.OnSendData(dwIoSize);
	_handle_send(BASIC_NETCODE_SUCC, dwIoSize);
	//每次发送重置idle
	m_unIdleCount = 0;
}

Net_Int CBasicSessionNetClient::OnReceive(Net_UInt dwNetCode, const char *pszData, Net_Int cbData)
{
	Net_Int lRet = _handle_rece(BASIC_NETCODE_SUCC, pszData, cbData);
	//返回 BASIC_NET_HR_RET_HANDSHAKE，表示握手成功。对于需要握手的连接，一定要在接收数据的处理函数里面返回这个值。
	if ((lRet & BASIC_NET_HR_RET_HANDSHAKE) && !(lRet & NET_ERROR))
	{
		SetSessionStatus(TIL_SS_SHAKEHANDLE_TRANSMIT, TIL_SS_SHAKEHANDLE_MASK);
	}
	return lRet;
}
Net_Int CBasicSessionNetClient::OnIdle(Net_UInt dwIdleCount)
{
	_handle_idle(dwIdleCount);
	return BASIC_NET_OK;
}

void CBasicSessionNetClient::OnReceiveData(const char* pszData, Net_UInt dwIoSize)
{
	if (dwIoSize == 0)
	{
		Close(TRUE);
		return;
	}

	int nReceived = (int)dwIoSize;
	if (nReceived < 0)
	{
		ASSERT(0);
		return;
	}

	Net_Int lRet = 0;
	m_stNet.OnReceiveData(nReceived);
	if (m_pPreSend != NULL)
	{
		PreReceiveData(BASIC_NETCODE_SUCC, pszData, nReceived);
		return;
	}
	OnReceive(BASIC_NETCODE_SUCC, pszData, nReceived);
}

Net_Int CBasicSessionNetClient::PreReceiveData(Net_UInt dwNetCode, const char *pszData, Net_Int cbData)
{
	const char* pPack = pszData;
	long lPackLen = cbData;
	int rRet = PACK_FILTER_SEARCH;
	m_bufCacheTmp.SetDataLength(0);
	rRet = m_pPreSend->OnPreReceive(pPack, lPackLen, m_bufCacheTmp, this);
	while (rRet == PACK_FILTER_NEXT)
	{
		//同一次接收，响应多次消息
		Net_Int lRecRet = OnReceive(dwNetCode | BASIC_NETCODE_FILTER_HANDLE, m_bufCacheTmp.GetDataBuffer(), m_bufCacheTmp.GetDataLength());
		if (lRecRet < BASIC_NET_OK)	//收到错误包后可能被断开
			return lRecRet;
		m_bufCacheTmp.SetDataLength(0);
		rRet = m_pPreSend->OnPreReceive(NULL, 0, m_bufCacheTmp, this);
	}
	if (rRet == PACK_FILTER_SKIP)
	{
		return OnReceive(dwNetCode, pPack, lPackLen);
	}
	else if (rRet == PACK_FILTER_HANDLED)
	{
		//do nothing
	}
	else if (rRet == PACK_FILTER_SEARCH)
	{
		pPack = m_bufCacheTmp.GetDataBuffer(lPackLen);
		dwNetCode |= BASIC_NETCODE_FILTER_HANDLE;
		return OnReceive(dwNetCode, pPack, lPackLen);
	}
	else
	{
		_handle_error(dwNetCode | BASIC_NETCODE_FILTER_ERROR, BASIC_NET_FILTER_READ_FAIL);
		return BASIC_NET_FILTER_READ_FAIL;
	}

	return BASIC_NET_OK;
}

void CBasicSessionNetClient::ResetPreSend()
{
	if (m_pPreSend != NULL)
	{
		m_pPreSend->ResetPreSend();
	}
}

void CBasicSessionNetClient::OnReadEvent(void)
{
	if (IsConnected())
	{
		char szBuf[16384];
		int nReceived = 0;
		while ((nReceived = recv(m_socketfd, szBuf, sizeof(szBuf), 0)) > 0)
		{
			OnReceiveData(szBuf, nReceived);
		}
		if (nReceived == 0)
		{
			Close(TRUE);
			return;
		}
		else if (nReceived < 0)
		{
			int nNumber = errno;
			if (nNumber == EAGAIN || nNumber == EINTR || errno == EWOULDBLOCK)
			{
				//加readevent
				event_add(&m_revent, NULL);
				return;
			}
			else
			{
				BOOL bError = FALSE;
#ifdef __BASICWINDOWS
				if (nReceived == SOCKET_ERROR)
				{
					DWORD dwLastError = WSAGetLastError();   //I know this
					if (dwLastError == WSAEWOULDBLOCK)
					{
						//加readevent
						event_add(&m_revent, NULL);
					}
					else if (dwLastError != WSA_IO_PENDING)
					{
						bError = TRUE;
					}
				}
#else
				bError = TRUE;
#endif
				if (bError){
					Close(TRUE);
					return;
				}
			}
		}
	}
}

void CBasicSessionNetClient::OnWriteEvent()
{
	Net_UInt dwStatus = GetSessionStatus(TIL_SS_LINK);
	if (dwStatus == TIL_SS_CONNECTING)
	{
		int nErr = 0;
		socklen_t nLen = sizeof(nErr);
		getsockopt(m_socketfd, SOL_SOCKET, SO_ERROR, (char *)&nErr, &nLen);
		Net_UInt dwNetCode = 0;
		if (nErr == 0)
		{
			dwNetCode = BASIC_NETCODE_SUCC;
		}
		OnConnect(dwNetCode);
	}
	else
	{
		SendDataFromQueue();
	}
}

Net_Int CBasicSessionNetClient::_handle_connect(Net_UInt dwNetCode)
{
	Net_Int lRet = BASIC_NET_OK;
	if (m_funcConnect)
	{
		CRefBasicSessionNet pRef = m_refSelf;
		lRet = m_funcConnect(this, dwNetCode);
	}
	return lRet;
}
Net_Int CBasicSessionNetClient::_handle_disconnect(Net_UInt dwNetCode)
{
	Net_Int lRet = BASIC_NET_OK;
	if (m_funcDisconnect)
	{
		CRefBasicSessionNet pRef = m_refSelf;
		lRet = m_funcDisconnect(this, dwNetCode);
	}
	return lRet;
}

Net_Int CBasicSessionNetClient::_handle_idle(Net_UInt dwNetCode)
{
	Net_Int lRet = BASIC_NET_OK;
	if (m_funcIdle)
	{
		CRefBasicSessionNet pRef = m_refSelf;
		lRet = m_funcIdle(this, dwNetCode);
	}
	return lRet;
}
Net_Int CBasicSessionNetClient::_handle_error(Net_UInt dwNetCode, Net_Int lRetCode)
{
	Net_Int lRet = BASIC_NET_OK;
	if (m_funcError)
	{
		CRefBasicSessionNet pRef = m_refSelf;
		lRet = m_funcError(this, dwNetCode, lRetCode);
	}
	return lRet;
}
Net_Int CBasicSessionNetClient::_handle_rece(Net_UInt dwNetCode, const char *pszData, Net_Int cbData)
{
	Net_Int lRet = BASIC_NET_OK;
	if (m_funcReceive)
	{
		CRefBasicSessionNet pRef = m_refSelf;
		lRet = m_funcReceive(this, dwNetCode, cbData, pszData);
	}
	return lRet;
}
Net_Int CBasicSessionNetClient::_handle_send(Net_UInt dwNetCode, Net_Int cbSend)
{
	Net_Int lRet = BASIC_NET_OK;
	if (m_funcSend)
	{
		CRefBasicSessionNet pRef = m_refSelf;
		lRet = m_funcSend(this, dwNetCode, cbSend);
	}
	return lRet;
}

__NS_BASIC_END

