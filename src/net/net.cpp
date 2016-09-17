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

/////////////////////////////////////////////////////////////////////////////
//net mgr
class CBasicNetMgv : public CBasicObject
{
public:
	CBasicNetMgv()
	{
		//create net
		CBasicSessionNet::Initialize(g_GetParamFunc);
	}
	virtual ~CBasicNetMgv()
	{
		//release net
		CBasicSessionNet::CloseSocket();
	}
};

//定义单态
typedef CBasicSingleton<CBasicNetMgv>	CBasicSingletonNetMgv;

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

struct CEventQueueItem
{
	CBasicSessionNet*								m_pRefNetSession;
	CBasicSessionNet::pCallSameRefNetSessionFunc	m_pCallFunc;
	Net_PtrInt										m_lRevert;
};
///////////////////////////////////////////////////////////////////////////////////////////////////
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
	void SetEvent(CBasicSessionNet* pSession, CBasicSessionNet::pCallSameRefNetSessionFunc pCallFunc, Net_PtrInt lRevert, bool bWait = false)
	{
		if (!bWait && IsEventThread())
		{
			SetSameThreadEvent(pSession, pCallFunc, lRevert);
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
		if (pCallFunc)
		{
			pCallFunc(pSession, lRevert);
		}
	}

public:
	DWORD				m_dwThreadID;
	evutil_socket_t		m_pair[2];
	struct event_base*	m_base;
	struct event		notify_event;
};

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
	}
	while (nReadLength != sizeof(CEventQueueItem));
}

THREAD_RETURN WorkerThread(void *arg)
{
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
	g_bTimeToKill = TRUE;
	if (g_nEventThreadCount > 0 && g_pEventThreads != nullptr)
	{
		for (int i = 0; i < g_nEventThreadCount; i++)
		{
			CNetThread* pThread = &g_pEventThreads[i];
			event_base_loopexit(pThread->m_base, nullptr);
		}
		int nTimes = 0;
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
#ifdef __BASICWINDOWS
	WSACleanup();
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CBasicSessionNet::CBasicSessionNet(Net_UInt nSessionID)
{
	//init net
	CBasicSingletonNetMgv::Instance();
	m_refSelf = this;
	m_pThread = &g_pEventThreads[nSessionID % g_nEventThreadCount];
	m_pPreSend = nullptr;
	InitMember();
}

void CBasicSessionNet::InitMember()
{
	m_socketfd = -1;
	m_unSessionStatus = 0;
}

CBasicSessionNet::~CBasicSessionNet()
{
	ASSERT(m_refSelf.GetResFunc() == nullptr);
	if (m_pPreSend != NULL)
	{
		delete m_pPreSend;
	}
}

void CBasicSessionNet::Release()
{
	SetLibEvent([](CBasicSessionNet* pSession, Net_PtrInt lRevert)->void{
		pSession->ReleaseCallback();
	}, 0, true);	
}

void CBasicSessionNet::ReleaseCallback()
{
	if (m_refSelf == nullptr)
		return;
	CloseCallback(FALSE);
	m_refSelf = nullptr;
}

void CBasicSessionNet::Close(BOOL bRemote)
{
	SetToClose();
	//must be wait, may be loop
	SetLibEvent([](CBasicSessionNet* pSession, Net_PtrInt lRevert)->void{
		BOOL bRemote = lRevert;
		pSession->CloseCallback(bRemote);
	}, bRemote, true);
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

void CBasicSessionNet::SetLibEvent(pCallSameRefNetSessionFunc pCallback, Net_PtrInt lRevert, bool bWait)
{
	m_pThread->SetEvent(this, pCallback, lRevert, bWait);
}

void CBasicSessionNet::CloseCallback(BOOL bRemote)
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
	m_sessionIDMgr = 0;
}

CBasicSessionNetServer::~CBasicSessionNetServer()
{

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
	CBasicSessionNetClient* pAcceptSession = CreateServerClientSession(basiclib::BasicInterlockedIncrement((LONG*)&m_sessionIDMgr));
	AcceptToSelf(pAcceptSession, s, addr);

	basiclib::CSingleLock lock(&m_mtxCSession, TRUE);
	m_mapClientSession[pAcceptSession->GetSessionID()] = pAcceptSession;
}

CBasicSessionNetClient* CBasicSessionNetServer::CreateServerClientSession(Net_UInt nSessionID)
{
	CBasicSessionNetClient* pNotify = ConstructSession(nSessionID);
	pNotify->bind_rece(m_funcReceive);
	pNotify->bind_send(m_funcSend);
	pNotify->bind_connect(m_funcConnect);
	pNotify->bind_disconnect([&](CBasicSessionNet* p1, Net_UInt p2)->Net_Int{
		//disconnect
		Net_Int lRet = m_funcDisconnect ? m_funcDisconnect(p1, p2) : BASIC_NET_OK;

		CBasicSessionNetClient* pClient = (CBasicSessionNetClient*)p1;
		//delete
		basiclib::CSingleLock lock(&m_mtxCSession, TRUE);
		m_mapClientSession.erase(pClient->GetSessionID());
		lock.Unlock();
		//release
		pClient->Release();
		return lRet;
	});
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
	return CBasicSessionNetClient::CreateClient(nSessionID);
}


//外部ontimer驱动1s
void CBasicSessionNetServer::OnTimer(int nTick)
{
	VTClientSession vtClient;
	CopyClientSession(vtClient);
	for (auto& client : vtClient)
	{
		client->OnTimer();
	}
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
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void OnLinkRead(int fd, short event, void *arg)
{
	CBasicSessionNetClient* pLink = (CBasicSessionNetClient*)arg;
	if (pLink && pLink->m_socketfd == fd)
	{
		pLink->OnReadEvent();
	}
}

void OnLinkWrite(int fd, short event, void *arg)
{
	CBasicSessionNetClient *pLink = (CBasicSessionNetClient *)arg;
	if (pLink && pLink->m_socketfd == fd)
	{
		pLink->OnWriteEvent();
	}
}


CBasicSessionNetClient::CBasicSessionNetClient(Net_UInt nSessionID) : CBasicSessionNet(nSessionID)
{
	m_nSessionID = nSessionID;
	m_usTimeoutShakeHandle = 10;
	m_unIdleCount = 0;
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
}

void CBasicSessionNetClient::CloseCallback(BOOL bRemote)
{
	if (m_socketfd != -1)
	{
		event_del(&m_wevent);
		CBasicSessionNet::CloseCallback(bRemote);

		Net_UInt dwNetCode = 0;
		if (bRemote)
		{
			dwNetCode |= BASIC_NETCODE_CLOSE_REMOTE;
		}
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
	event_add(&m_revent, NULL);
}

Net_Int CBasicSessionNetClient::OnConnect(Net_UInt dwNetCode)
{
	if (dwNetCode & BASIC_NETCODE_SUCC)
	{
		SetSessionStatus(TIL_SS_CONNECTED, TIL_SS_LINK);
		m_stNet.Empty();
	}
	else
	{
		Close(TRUE);
		return BASIC_NET_GENERIC_ERROR;
	}

	ResetPreSend();

	Net_Int lRet = _handle_connect(dwNetCode);
	//如果函数返回 BASIC_NET_HC_RET_HANDSHAKE，表示该连接需要进行握手。否则认为握手成功。
	if (!(lRet & BASIC_NET_HC_RET_HANDSHAKE))
	{
		SetSessionStatus(TIL_SS_SHAKEHANDLE_TRANSMIT, TIL_SS_SHAKEHANDLE_MASK);
	}
	return lRet;
}

//out timer
void CBasicSessionNetClient::OnTimer()
{
	_OnIdle();
}

Net_Int CBasicSessionNetClient::OnDisconnect(Net_UInt dwNetCode)
{
	_handle_disconnect(dwNetCode);
	return BASIC_NET_OK;
}

void CBasicSessionNetClient::_OnIdle()
{
	if (GetSessionStatus(TIL_SS_LINK) == TIL_SS_CONNECTED)
	{
		m_unIdleCount++;
		if (GetSessionStatus(TIL_SS_SHAKEHANDLE_MASK) != TIL_SS_SHAKEHANDLE_TRANSMIT && m_unIdleCount > m_usTimeoutShakeHandle)
		{
			Close();
			return;
		}
		OnIdle(m_unIdleCount);
	}
}

Net_Int CBasicSessionNetClient::DoConnect()
{
	Net_UInt dwLinkNetStatus = GetSessionStatus(TIL_SS_LINK);
	Net_UInt dwCloseNetStatus = GetSessionStatus(TIL_SS_CLOSE);
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
		else if (errno == EINPROGRESS)
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
		m_outBuffer.len = m_quSend.ReadBuffer(m_outBuffer.buf, MAX_BUFFER);
	}
	return m_outBuffer.len > 0;
}

BOOL CBasicSessionNetClient::SendDataFromQueue()
{
	basiclib::CSpinLockFunc lock(&m_lockSend);
	if (lock.LockNoWait())
	{
		if (!IsConnected())
		{
			return FALSE;
		}
		BOOL bError = FALSE;
		BOOL bFull = FALSE;
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
					SetLibEvent([](CBasicSessionNet* pSession, Net_PtrInt lRevert)->void{
						((CBasicSessionNetClient*)pSession)->AddWriteEvent();
					});
					bFull = TRUE;
					break;
				}
				else if (nNumber == EINTR)
				{
					lSend = 0;
					continue;
				}
				else
				{
					bError = TRUE;
					break;
				}
			}
		}
		lock.UnLock();
		if (bError)
		{
			Close(TRUE);
		}
		else
		{
			OnSendData(nTotalSend);
			return !bFull;
		}
	}
	return FALSE;
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
	Net_Int lRet = m_quSend.AddSendBuffer((const char*)pData, cbData);
	if (lRet > 0)
	{
		while (SendDataFromQueue()){}
	}
	return lRet;
}

void CBasicSessionNetClient::OnSendData(Net_UInt dwIoSize)
{
	if (IsToClose() || !IsConnected())
		return;
	m_stNet.OnSendData(dwIoSize);
	_handle_send(BASIC_NETCODE_SUCC, dwIoSize);
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
	CBasicSmartBuffer bufTmp;
	rRet = m_pPreSend->OnPreReceive(pPack, lPackLen, bufTmp, this);
	while (rRet == PACK_FILTER_NEXT)
	{
		//同一次接收，响应多次消息
		Net_Int lRecRet = OnReceive(dwNetCode | BASIC_NETCODE_FILTER_HANDLE, bufTmp.GetDataBuffer(), bufTmp.GetDataLength());
		if (lRecRet < BASIC_NET_OK)	//收到错误包后可能被断开
			return lRecRet;
		bufTmp.Free();
		rRet = m_pPreSend->OnPreReceive(NULL, 0, bufTmp, this);
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
		pPack = bufTmp.GetDataBuffer(lPackLen);
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
		while (SendDataFromQueue()){}
	}
}

__NS_BASIC_END

