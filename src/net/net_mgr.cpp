#include "../inc/basic.h"
#include "net_mgr.h"

__NS_BASIC_START

CBasicString DefaultParamFuc(const char* pParam) {
	return "";
}

CBasicNetMgv* m_gNetMgrPoint = nullptr;
pGetConfFunc g_GetParamFunc = DefaultParamFuc;

#ifdef BASICWINDOWS_USE_IOCP
#include <mswsock.h>
CNetThread*	g_pEventThreads = nullptr;		//完成端口
int			g_nEventThreadCount = 1;
int			g_nIntThreadCount = 0;
bool CNetThread::m_bExtInit = false;
IOCPExt_Func CNetThread::m_funcExt;
#else
#include <netinet/in.h>
CNetThread*	g_pEventThreads = nullptr;
int			g_nEventThreadCount = 1;
int			g_nIntThreadCount = 0;
///////////////////////////////////////////////////////////////////////////////
void ReadSelfOrder(int fd, short event, void *arg){
	CNetThread *thr = (CNetThread*)arg;

	char szBuf[READBUFFERSIZE_MSG];
	int nReceived = 0;
	while((nReceived = recv(thr->m_pair[0], szBuf, READBUFFERSIZE_MSG, 0)) == READBUFFERSIZE_MSG){
	}
	thr->RunMessageQueue();
}
THREAD_RETURN WorkerThread(void *arg){
	srand((unsigned int)(time(NULL) + BasicGetTickTime() + BasicGetCurrentThreadId()));
	CNetThread *thr = (CNetThread*)arg;
	BasicInterlockedIncrement((LONG*)&g_nIntThreadCount);
	event_base_loop(thr->m_base, 0);
	BasicInterlockedDecrement((LONG*)&g_nIntThreadCount);
	return 0;
}
#endif

/////////////////////////////////////////////////////////////////////////////
CBasicNetMgv::CBasicNetMgv() {
	m_gNetMgrPoint = this;
	m_bTimeToKill = TRUE;
	m_bTimerStop = false;
	//create net
	Initialize(g_GetParamFunc);
	m_vtAddList.reserve(256);
	m_vtAddListDeal.reserve(256);
	m_vtDelList.reserve(256);
	m_vtDelListDeal.reserve(256);
	m_vtDeathSession.reserve(256);
	m_vtDeathSessionDeal.reserve(256);
}

CBasicNetMgv::~CBasicNetMgv() {
	//release net
	CloseNetSocket();
}

THREAD_RETURN CBasicNetMgv::ThreadCheckFunc(void* lpWorkContext) {
	srand((unsigned int)(time(NULL) + BasicGetTickTime() + BasicGetCurrentThreadId()));
	m_gNetMgrPoint->OnTimer();
	return 0;
}
#ifdef BASICWINDOWS_USE_IOCP
void CBasicNetMgv::Initialize(pGetConfFunc func){
	m_bTimeToKill = FALSE;

	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);

	g_nEventThreadCount = atol(func("NetThreadCount").c_str());
	if(g_nEventThreadCount <= 0)
		//default one thread
		g_nEventThreadCount = 1;
	g_pEventThreads = new CNetThread[g_nEventThreadCount];

	//init all thread
	while(g_nEventThreadCount != g_nIntThreadCount){
		basiclib::BasicSleep(100);
	}
	DWORD nCheckThreadID = 0;
	BasicCreateThread(ThreadCheckFunc, NULL, &nCheckThreadID);
}
#else
void CBasicNetMgv::Initialize(pGetConfFunc func) {
	m_bTimeToKill = FALSE;

	//使用自己的内存分配
	event_set_mem_functions(BasicAllocate, BasicReallocate, BasicDeallocate);
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
#endif
	evdns_set_log_fn([](int is_warn, const char *msg) {
		BasicLogEventV(DebugLevel_Info, "%s: %s", is_warn ? "WARN" : "INFO", msg);
	});

	g_nEventThreadCount = atol(func("NetThreadCount").c_str());
	if (g_nEventThreadCount <= 0)
		//default one thread
		g_nEventThreadCount = 1;
	g_pEventThreads = new CNetThread[g_nEventThreadCount];

	//windows no multithread
	for (int i = 0; i < g_nEventThreadCount; i++)
	{
		CNetThread* pThread = &g_pEventThreads[i];

		if (evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, pThread->m_pair) == -1){
			BasicLogEventError("create evutil_socketpair error");
			exit(1);
		}
		//写入是阻塞，读取非阻塞
		evutil_make_socket_nonblocking(pThread->m_pair[0]);
		evutil_make_socket_nonblocking(pThread->m_pair[1]);

		//create self eventbase
		pThread->m_base = event_base_new_with_config(cfg);
		event_set(&pThread->notify_event, pThread->m_pair[0], EV_READ | EV_PERSIST, ReadSelfOrder, pThread);
		event_base_set(pThread->m_base, &pThread->notify_event);
		if (event_add(&pThread->notify_event, NULL) == -1){
			BasicLogEventError("libevent eventadd error");
			exit(1);
		}
		pThread->m_dnsbase = evdns_base_new(pThread->m_base, 1);
		if (!pThread->m_dnsbase) {
			BasicLogEventError("libevent eventdns add error");
			exit(1);
		}
		BasicCreateThread(WorkerThread, pThread, &(pThread->m_dwThreadID));
	}
	//init all thread
	while (g_nIntThreadCount != g_nEventThreadCount){
		basiclib::BasicSleep(100);
	}
	DWORD nCheckThreadID = 0;
	BasicCreateThread(ThreadCheckFunc, NULL, &nCheckThreadID);

	event_config_free(cfg);
}
#endif

void CBasicNetMgv::CloseNetSocket(){
	TRACE("Start CloseSocket!");
	m_bTimeToKill = TRUE;
	if(g_nEventThreadCount > 0 && g_pEventThreads != nullptr){
		int nTimes = 0;
		while(!m_gNetMgrPoint->m_bTimerStop){
			nTimes++;
			BasicSleep(100);
			if(nTimes > 100){
				TRACE("CloseSocket timer no close!");
				break;
			}
		}
		for(int i = 0; i < g_nEventThreadCount; i++){
			CNetThread* pThread = &g_pEventThreads[i];
			pThread->ReadyToClose();
		}
#ifdef __BASICWINDOWS
		WSACleanup();
#endif
		nTimes = 0;
		while(g_nIntThreadCount != 0){
			nTimes++;
			BasicSleep(100);
			if(nTimes > 100){
				TRACE("CloseSocket thread no close!");
				break;
			}
		}
		delete[] g_pEventThreads;
		g_pEventThreads = nullptr;
	}
	TRACE("End CloseSocket!");
}

//! 加入timer
void CBasicNetMgv::AddToTimer(CBasicNet_Socket* pSocket) {
	CSpinLockFuncNoSameThreadSafe lock(&m_spinLockAdd, TRUE);
	m_vtAddList.push_back(pSocket);
}
//! 删除timer
void CBasicNetMgv::DelToTimer(CBasicNet_Socket* pSocket){
	CSpinLockFuncNoSameThreadSafe lock(&m_spinLockAdd, TRUE);
	m_vtDelList.push_back(pSocket);
	m_vtDeathSession.push_back(pSocket->GetRealSessionNet());
}

void CBasicNetMgv::OnTimer() {
	static unsigned int g_nTick = 0;
	while (!m_bTimeToKill) {
		//! 加入ontimer
		{
			CSpinLockFuncNoSameThreadSafe lock(&m_spinLockAdd, TRUE);
			swap(m_vtAddListDeal, m_vtAddList);
			swap(m_vtDelListDeal, m_vtDelList);
			swap(m_vtDeathSessionDeal, m_vtDeathSession);
		}
		for (auto&session : m_vtAddListDeal) {
			m_vtOnTimerList.push_back(session);
		}
		m_vtAddListDeal.clear();
		for(auto&session : m_vtDelListDeal){
			CBasicNetMgv::VTOnTimerSessionList::iterator iter = find(m_vtOnTimerList.begin(), m_vtOnTimerList.end(), session);
			if(iter != m_vtOnTimerList.end()){
				m_vtOnTimerList.erase(iter);
			}
		}
		m_vtDelListDeal.clear();
#ifdef _DEBUG
		if(m_vtDeathSessionDeal.size() > 0){
			m_vtDeathSessionDeal.clear();
		}
#endif
		m_vtDeathSessionDeal.clear();

		//! 执行ontimer
		long lCount = 0;
		for(auto&session : m_vtOnTimerList){
			session->OnTimer(g_nTick);
			lCount++;
			if(!(lCount % 1000)){
				basiclib::BasicSleep(100);		//I know this
			}
		}
		g_nTick++;
		basiclib::BasicSleep(1000);		//I know this
	}
	m_bTimerStop = true;
}
/////////////////////////////////////////////////////////////////////////////
#ifdef BASICWINDOWS_USE_IOCP
static void *get_extension_function(SOCKET s, const GUID *which_fn){
	void *ptr = NULL;
	DWORD bytes = 0;
	WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,
		(GUID*)which_fn, sizeof(*which_fn),
			 &ptr, sizeof(ptr),
			 &bytes, NULL, NULL);

	/* No need to detect errors here: if ptr is set, then we have a good
	function pointer.  Otherwise, we should behave as if we had no
	function pointer.
	*/
	return ptr;
}
#endif

CNetThread::CNetThread() {
	m_dwThreadID = 0;
	m_smBuf.SetDataLength(1024 * sizeof(intptr_t));
	m_smBuf.SetDataLength(0);
	m_smRunBuf.SetDataLength(1024 * sizeof(intptr_t));
	m_smRunBuf.SetDataLength(0);
#ifdef BASICWINDOWS_USE_IOCP
	m_ulFlags = 0;
	m_hCompletionPort = CreateIoCompletionPort((HANDLE)INVALID_SOCKET, NULL, 0, 0);
	if(m_hCompletionPort == NULL){
		BasicLogEventError("m_hCompletionPort create error");
		exit(1);
	}
	UINT nThreadID = 0;
	HANDLE hWorker = (HANDLE)_beginthreadex(nullptr, 0, ThreadIOCPFunc, this, 0, &(nThreadID));
	ASSERT(hWorker != NULL);
	CloseHandle(hWorker);		//I know this
	m_olFunc.m_ioType = IOFunc;

	if(!m_bExtInit){
		m_bExtInit = true;
		const GUID acceptex = WSAID_ACCEPTEX;
		const GUID connectex = WSAID_CONNECTEX;
		const GUID getacceptexsockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
		SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
		if(s == INVALID_SOCKET){
			BasicLogEventError("ext func get error");
			exit(1);
		}
		m_funcExt.AcceptEx = (AcceptExPtr)get_extension_function(s, &acceptex);
		m_funcExt.ConnectEx = (ConnectExPtr)get_extension_function(s, &connectex);
		m_funcExt.GetAcceptExSockaddrs = (GetAcceptExSockaddrsPtr)get_extension_function(s, &getacceptexsockaddrs);
		closesocket(s);
	}
#else
	m_base = nullptr;
	m_dnsbase = nullptr;
#endif
}

CNetThread::~CNetThread() {
#ifndef BASICWINDOWS_USE_IOCP
	if (m_dnsbase) {
		evdns_base_free(m_dnsbase, 0);
	}
	if (m_base) {
		event_del(&notify_event);
		evutil_closesocket(m_pair[0]);
		evutil_closesocket(m_pair[1]);
		event_base_free(m_base);
	}
#endif
}

//! 发送事件
void CNetThread::SetEvent(CBasicNet_Socket* pSession, CBasicNet_Socket::pCallSameRefNetSessionFunc pCallFunc, intptr_t lRevert){
	if(BasicGetCurrentThreadId() == m_dwThreadID){
		pCallFunc(pSession, lRevert);
		return;
	}
	CEventQueueItem eventQueue;
	eventQueue.m_pRefNetSession = pSession;
	eventQueue.m_pCallFunc = pCallFunc;
	eventQueue.m_lRevert = lRevert;
	//增加引用
	pSession->GetRealSessionNet()->AddRef();
	pSession->AddSocketCallFunc(&eventQueue);
}

void CNetThread::ReadyToClose(){
#ifdef BASICWINDOWS_USE_IOCP
	PostQueuedCompletionStatus(m_hCompletionPort, 0, (DWORD)NULL, NULL);		//I know this
#else
	event_base_loopbreak(m_base);
#endif
}


//! 加入到全局消息队列
void CNetThread::AddMessageQueue(CBasicNet_Socket* pSocket){
	long lLength = 0;
	{
		//增加引用
		pSocket->GetRealSessionNet()->AddRef();
		basiclib::CSpinLockFuncNoSameThreadSafe lock(&m_lockMsg, TRUE);
		lLength = m_smBuf.GetDataLength();
		m_smBuf.AppendDataEx((const char*)&pSocket, sizeof(CBasicNet_Socket*));
	}
	if(lLength == 0){
#ifdef BASICWINDOWS_USE_IOCP
		PostQueuedCompletionStatus(m_hCompletionPort, 0, (DWORD)nullptr, &m_olFunc.m_ol);  //I know this
#else
		send(m_pair[1], "", 1, 0);
#endif
	}
}
//! 处理消息队列
void CNetThread::RunMessageQueue(){
	static int g_nRunMessageQueueSize = sizeof(CBasicNet_Socket*);
	{
		basiclib::CSpinLockFuncNoSameThreadSafe lock(&m_lockMsg, TRUE);
		swap(m_smRunBuf, m_smBuf);
	}
	int nCount = m_smRunBuf.GetDataLength() / g_nRunMessageQueueSize;
	if(nCount > 0){
		char* pBegin = m_smRunBuf.GetDataBuffer();
		for(int i = 0; i < nCount; i++){
			CBasicNet_Socket* pItem = *(CBasicNet_Socket**)(pBegin + i * g_nRunMessageQueueSize);
			pItem->RunSocketCallFunc();
			pItem->GetRealSessionNet()->DelRef();
		}
		m_smRunBuf.SetDataLength(0);
	}
}

#ifdef BASICWINDOWS_USE_IOCP
unsigned CNetThread::ThreadIOCPFunc(void* lpWorkContext){
	srand((unsigned int)(time(NULL) + BasicGetTickTime() + BasicGetCurrentThreadId()));
	BasicInterlockedIncrement((LONG*)&g_nIntThreadCount);

	CNetThread* pMgr = (CNetThread*)lpWorkContext;
	pMgr->m_dwThreadID = basiclib::BasicGetCurrentThreadId();
	HANDLE hCompletionPort = pMgr->m_hCompletionPort;
	OVERLAPPED *overlapped = nullptr;
	OVERLAPPEDPLUS* pOverlapPlus = nullptr;
	DWORD& dwIoSize = pMgr->m_dwIoSize;
	CBasicNet_Socket* lpTcpLink = nullptr;
	ULONG& ulFlags = pMgr->m_ulFlags;
	while(!m_gNetMgrPoint->m_bTimeToKill){
		overlapped = nullptr;
		dwIoSize = 0;
		lpTcpLink = nullptr;
		pOverlapPlus = nullptr;
		BOOL bIORet = GetQueuedCompletionStatus(hCompletionPort, &dwIoSize, (LPDWORD)&lpTcpLink, &overlapped, INFINITE);
		if(overlapped){
			if(bIORet){
				pOverlapPlus = CONTAINING_RECORD(overlapped, OVERLAPPEDPLUS, m_ol);
				switch(pOverlapPlus->m_ioType){
				case IOFunc:
				{
					pMgr->RunMessageQueue();
					break;
				}
				case IORead:
				{
					CBasicNet_SocketTransfer* pSocket = (CBasicNet_SocketTransfer*)lpTcpLink;
					if(dwIoSize == 0){
						lpTcpLink->Close(true);
					}
					else{
						int nReceived = (int)dwIoSize;
						if(nReceived > 0){
							pSocket->OnReceiveData(pSocket->m_byInBuffer, dwIoSize);
							if(lpTcpLink->GetSocketID() > 0){
								pMgr->StartRecvData(pSocket);
							}
						}
					}
					break;
				}
				case IOWrite:
				{
					((CBasicNet_SocketTransfer*)lpTcpLink)->SendDataSuccessAndCheckSend(dwIoSize);
					break;
				}
				case IOAccept:
				{
					lpTcpLink->ServerAcceptEx(pOverlapPlus);
					break;
				}
				case IOConnect:
				{
					lpTcpLink->ClientConnectEx();
					break;
				}
				default:
				{
					ASSERT(0);
				}
				}

			}
			else{
				DWORD dwIOError = GetLastError();  //I know this
				if(dwIOError != WAIT_TIMEOUT){
					lpTcpLink->Close(TRUE);
				}
			}
		}
};
	BasicInterlockedDecrement((LONG*)&g_nIntThreadCount);
	return 0;
}

//! 开始接收数据
bool CNetThread::StartRecvData(CBasicNet_SocketTransfer* pSocket){
	UINT nRetVal = WSARecv(pSocket->GetSocketID(), &pSocket->m_wsaInBuffer, 1, &m_dwIoSize, &m_ulFlags, &pSocket->m_olRead.m_ol, NULL);  //I know this
	if(nRetVal == SOCKET_ERROR){
		int nError = WSAGetLastError();
		if(nError != WSA_IO_PENDING){
			ASSERT(0);
			pSocket->Close();
			return false;
		}
	}
	return true;
}
#else
//! 异步dns解析
bool CNetThread::DNSParse(const char* pName, evdns_getaddrinfo_cb pCallback, CBasicNet_Socket* pSession) {
	struct  evutil_addrinfo  hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = EVUTIL_AI_CANONNAME;
	/* Unless we specify a socktype, we'llget at least two entries for
	* each address: one for TCP and onefor UDP. That's not what we
	* want. */
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	return evdns_getaddrinfo(
		m_dnsbase, pName, NULL /* no service name given */,
		&hints, pCallback, pSession) != nullptr;
}
#endif

__NS_BASIC_END