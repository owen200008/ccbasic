#include "../inc/basic.h"
#include "net_mgr.h"
#include "net_server.h"
#ifdef __BASICWINDOWS
#elif defined(__LINUX)
#include <signal.h>
#endif

#if defined(__LINUX) || defined(__MAC) || defined(__ANDROID)
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <netinet/tcp.h>
extern void OnLinkRead(evutil_socket_t fd, short event, void *arg);
extern void OnLinkWrite(evutil_socket_t fd, short event, void *arg);
#endif

__NS_BASIC_START

extern int			g_nEventThreadCount;
////////////////////////////////////////////////////////////////////////////////////////////////////////
class CBasicNet_SocketSession : public CBasicNet_SocketTransfer
{
public:
	CBasicNet_SocketSession(CBasicSessionNetServerSession* pFather, uint32_t nSessionID, uint16_t usRecTimeout = 0);
	virtual ~CBasicNet_SocketSession();

	//!获取对端的地址和端口
	const char* GetNetAddress() { return m_szPeerAddr; }
	uint32_t GetNetAddressPort() { return m_nPeerPort; }

	//! 获取状态
	virtual void GetNetStatus(CBasicString& strStatus);
protected:
	//! server线程内
	void Accept(evutil_socket_t s, sockaddr* pAddr);

	//! 初始化事件
	bool InitServerSessionEvent();
protected:
	char					m_szPeerAddr[ADDRESS_MAX_LENGTH];
	uint32_t				m_nPeerPort;

	friend class CBasicNet_SocketListen;
};

CBasicSessionNetServerSession* CreateNetWithServerSession(size_t nClassSize, uint32_t nSessionID, uint16_t usRecTimeout, const std::function<CBasicSessionNetServerSession*(void*)>& func){
	void* pData = basiclib::BasicAllocate(nClassSize + sizeof(CBasicNet_SocketSession));
	CBasicSessionNetServerSession* pClient = func(pData);
	pClient->InitSocket(new ((char*)pData + nClassSize) CBasicNet_SocketSession(pClient, nSessionID, usRecTimeout));
	return pClient;
}

CBasicNet_SocketSession::CBasicNet_SocketSession(CBasicSessionNetServerSession* pFather, uint32_t nSessionID, uint16_t usRecTimeout) : CBasicNet_SocketTransfer(pFather, nSessionID, usRecTimeout){
	memset(m_szPeerAddr, 0, sizeof(m_szPeerAddr));
	m_nPeerPort = 0;
}
CBasicNet_SocketSession::~CBasicNet_SocketSession(){
}

//! 获取状态
void CBasicNet_SocketSession::GetNetStatus(CBasicString& strStatus){
	uint32_t dwLinkNetStatus = GetSessionStatus(TIL_SS_LINK);
	uint32_t dwCloseNetStatus = GetSessionStatus(TIL_SS_CLOSE);
	char szBuf[32] = { 0 };
	if(dwCloseNetStatus != TIL_SS_NORMAL){
		switch(dwCloseNetStatus){
		case TIL_SS_TOCLOSE:
		{
			strcpy(szBuf, "TOCLOSE");
		}
		break;
		}
	}
	else{
		switch(dwLinkNetStatus){
		case TIL_SS_IDLE:
			strcpy(szBuf, "IDLE");
			break;
		case TIL_SS_CONNECTING:
			strcpy(szBuf, "CONNECTING");
			break;
		case TIL_SS_CONNECTED:
		{
			if(GetSessionStatus(TIL_SS_SHAKEHANDLE_MASK) != TIL_SS_SHAKEHANDLE_TRANSMIT)
				strcpy(szBuf, "CONNECTED");
			else
				strcpy(szBuf, "OK");
		}
		break;
		}
	}
	char szBufTime[32] = { 0 };
	if(m_stNet.m_tmLastRecTime > 0){
		basiclib::CTime tm(m_stNet.m_tmLastRecTime);
		tm.FormatToBuffer("%H:%M:%S", szBufTime, 32);
	}

	double dSendRate = 0;
	double dRecvRate = 0;
	m_stNet.GetTransRate(m_lastNet, dSendRate, dRecvRate);
	CBasicString strTemp;
	strTemp.Format("Link: %s LastReceive: %s Timeout: %d \r\nStatus: %s R%uK S%uK R:%.2fKB/s S:%.2fKB/s\t\r\n",
				   m_szPeerAddr, szBufTime, m_usRecTimeout, szBuf, m_stNet.m_dwReceBytes / 1024, m_stNet.m_dwSendBytes / 1024, dRecvRate, dSendRate);
	strStatus += strTemp;
}

/////////////////////////////////////////////////////////////////////////////////////
void CBasicNet_SocketSession::Accept(evutil_socket_t s, sockaddr* pAddr) {
	#if defined(__MAC)
	int set = 1;
	setsockopt(s, SOL_SOCKET, SO_NOSIGPIPE, (void*)&set, sizeof(int));
#endif
	SetSessionStatus(TIL_SS_CONNECTING, TIL_SS_LINK);

	int nFamily = pAddr->sa_family;
	if (nFamily == AF_INET) {
		char szBuf[128] = { 0 };
		sockaddr_in* pSockAddr = (struct sockaddr_in*)pAddr;
		m_nPeerPort = ntohs(pSockAddr->sin_port);		//I know this
		strncpy(m_szPeerAddr, evutil_inet_ntop(AF_INET, &pSockAddr->sin_addr, szBuf, sizeof(szBuf)), ADDRESS_MAX_LENGTH);		//I know this
	}
	else {
		char szBuf[128] = { 0 };
		sockaddr_in6* pSockAddr = (struct sockaddr_in6*)pAddr;
		m_nPeerPort = ntohs(pSockAddr->sin6_port);		//I know this
		strncpy(m_szPeerAddr, evutil_inet_ntop(AF_INET6, &pSockAddr->sin6_addr, szBuf, sizeof(szBuf)), ADDRESS_MAX_LENGTH);		//I know this
	}

	evutil_make_socket_nonblocking(s);
	evutil_make_listen_socket_reuseable(s);
	evutil_make_listen_socket_reuseable_port(s);

	//赋值socketid,保证同步，获取session
	m_socketfd = s;
	SetLibEvent([](CBasicNet_Socket* pNotify, intptr_t lRevert)->void {
		CBasicNet_SocketSession* pSession = (CBasicNet_SocketSession*)pNotify;
		if(pSession->InitServerSessionEvent())
			pSession->OnConnect(BASIC_NETCODE_SUCC);
		else
			pSession->OnConnect(BASIC_NETCODE_CLOSE_REMOTE);
	});
}

//! 初始化事件
bool CBasicNet_SocketSession::InitServerSessionEvent(){
#ifdef BASICWINDOWS_USE_IOCP
	//已经绑定到IOCP，开启receive
	return m_pThread->StartRecvData(this);
#else
	event_set(&m_revent, m_socketfd, EV_READ | EV_PERSIST, OnLinkRead, this);
	event_base_set(m_pThread->m_base, &m_revent);
	event_set(&m_wevent, m_socketfd, EV_WRITE, OnLinkWrite, this);
	event_base_set(m_pThread->m_base, &m_wevent);
	//read data
	event_add(&m_revent, NULL);
	return true;
#endif
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
//! 获取sessionid，等于socketid
uint32_t CBasicSessionNetServerSession::GetSessionID(){
	return m_pSocket->GetSocketID();
}

//!获取对端的地址和端口
const char* CBasicSessionNetServerSession::GetNetAddress(){ 
	return ((CBasicNet_SocketSession*)m_pSocket)->GetNetAddress();
}
uint32_t CBasicSessionNetServerSession::GetNetAddressPort() {
	return ((CBasicNet_SocketSession*)m_pSocket)->GetNetAddressPort();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef BASICWINDOWS_USE_IOCP
#include <MSWSock.h>
#endif
class CBasicNet_SocketListen : public CBasicNet_Socket {
public:
#ifdef BASICWINDOWS_USE_IOCP
	struct ServerAcceptExData{
		evutil_socket_t						m_socketfd;
		OVERLAPPEDPLUS						m_olAcceptEx;
		ADDRESS_FAMILY						m_createFamily;
		char*								m_pBuf;
		int									m_nBufLength;
		CBasicSessionNetServerSession*		m_pSession;
		ServerAcceptExData(){
			m_socketfd = INVALID_SOCKET;
			m_olAcceptEx.m_ioType = IOAccept;
			m_pBuf = nullptr;
			m_nBufLength = 0;
			m_createFamily = AF_UNSPEC;
		}
		~ServerAcceptExData(){
			if(m_pBuf){
				basiclib::BasicDeallocate(m_pBuf);
			}
			if(m_socketfd != INVALID_SOCKET){
				evutil_closesocket(m_socketfd);
				m_pSession->SafeDelete();
			}
		}
		void InitBuffer(sockaddr_storage& addr){
			if(m_createFamily == addr.ss_family)
				return;
			int addrlen = 0;
			if(addr.ss_family == AF_INET)
				addrlen = sizeof(struct sockaddr_in);
			else if(addr.ss_family == AF_INET6)
				addrlen = sizeof(struct sockaddr_in6);
			else{
				ASSERT(0);
				return;
			}
			m_createFamily = addr.ss_family;
			m_nBufLength = (addrlen + 16) * 2;
			if(m_pBuf){
				basiclib::BasicDeallocate(m_pBuf);
			}
			m_pBuf = (char*)basiclib::BasicAllocate(m_nBufLength);
		}
		void StartAccept(CBasicNet_SocketListen* pListen){
			if(m_socketfd != INVALID_SOCKET)
				return;
			SOCKET s = socket(m_createFamily, SOCK_STREAM, 0);
			if(s == INVALID_SOCKET){
				basiclib::BasicLogEventErrorV("%s(%s:%d) ERRER(%d)", __FUNCTION__, __FILE__, __LINE__, WSAGetLastError());
				return;
			}
			evutil_make_socket_nonblocking(s);

			evutil_socket_t& socketIDListen = pListen->GetSocketID();
			m_socketfd = s;
			m_pSession = pListen->m_pFather->CreateServerClientSession(pListen->m_nChildThread++);
			CNetThread* pIOCP = m_pSession->GetBasicNet_Socket()->GetSelfNetThread();
			CreateIoCompletionPort((HANDLE)m_socketfd, pIOCP->m_hCompletionPort, (DWORD)m_pSession->GetBasicNet_Socket(), g_nEventThreadCount);
			IOCPExt_Func& func = CNetThread::GetExtFunc();
			DWORD pending = 0;
			if(!func.AcceptEx(socketIDListen, m_socketfd, m_pBuf, 0, m_nBufLength / 2, m_nBufLength / 2, &pending, &m_olAcceptEx.m_ol)){
				int error = WSAGetLastError();
				if(error != ERROR_IO_PENDING){
					basiclib::BasicLogEventErrorV("%s(%s:%d) ERRER(%d)", __FUNCTION__, __FILE__, __LINE__, error);
					return;
				}
			}
		}
	};
#endif
public:
	CBasicNet_SocketListen(CBasicSessionNetServer* pServer);
	virtual ~CBasicNet_SocketListen();

	//! 判断是否监听
	bool IsListen() { return m_socketfd != INVALID_SOCKET; }

	//! 监听操作
	int32_t Listen(const char* lpszAddress, bool bWaitSuccess);
#ifdef BASICWINDOWS_USE_IOCP
	//! server监听异步accept使用
	virtual void ServerAcceptEx(OVERLAPPEDPLUS* pOverlapPlus){
		if(IsToClose()){
			Close();
			return;
		}
		ServerAcceptExData* pAcceptData = CONTAINING_RECORD(pOverlapPlus, ServerAcceptExData, m_olAcceptEx);

		struct sockaddr *sa_local = NULL, *sa_remote = NULL;
		int socklen_local = 0, socklen_remote = 0;
		IOCPExt_Func& func = CNetThread::GetExtFunc();
		func.GetAcceptExSockaddrs(pAcceptData->m_pBuf, 0, pAcceptData->m_nBufLength / 2, pAcceptData->m_nBufLength / 2,
								  &sa_local, &socklen_local, &sa_remote,
								  &socklen_remote);

		DoAcceptClient(pAcceptData->m_socketfd, pAcceptData->m_pSession, sa_remote);
		pAcceptData->m_socketfd = INVALID_SOCKET;
		pAcceptData->m_pSession = nullptr;
		pAcceptData->StartAccept(this);
	}
#endif
public:
	//! ontimer线程
	virtual bool OnTimer(unsigned int nTick);
protected:
	friend void OnLinkListenRead(evutil_socket_t fd, short event, void *arg);

	//! 监听初始化
	void InitListenEvent(evutil_socket_t socketfd);

	//! 接收客户端
	void AcceptClient();
	void DoAcceptClient(evutil_socket_t s, CBasicSessionNetServerSession* pAcceptSession, sockaddr* pAddr);
protected:
	virtual CBasicSessionNet* GetRealSessionNet() { return m_pFather; }
protected:
	uint32_t								m_nChildThread;		//分配线程
	CBasicSessionNetServer*					m_pFather;
	//waitlisten event
	basiclib::CEvent						m_eventListen;
#ifdef BASICWINDOWS_USE_IOCP
#define DEFAULT_SERVERACCEPTEXDATA_COUNT	4
	//默认启动4个acceptex
	ServerAcceptExData						m_acceptData[DEFAULT_SERVERACCEPTEXDATA_COUNT];
#endif
};

CBasicSessionNetServer* CreateNetWithServer(size_t nClassSize, const std::function<CBasicSessionNetServer*(void*)>& func){
	void* pData = basiclib::BasicAllocate(nClassSize + sizeof(CBasicNet_SocketListen));
	CBasicSessionNetServer* pServer = func(pData);
	pServer->InitSocket(new ((char*)pData + nClassSize) CBasicNet_SocketListen(pServer));
	return pServer;
}


CBasicNet_SocketListen::CBasicNet_SocketListen(CBasicSessionNetServer* pServer) : CBasicNet_Socket(CBasicNet_Socket::GetDefaultCreateSessionID()) {
	m_pFather = pServer;
	//随机值就可以
	//m_nChildThread = 0;
}

CBasicNet_SocketListen::~CBasicNet_SocketListen() {

}

//! 监听操作
int32_t CBasicNet_SocketListen::Listen(const char* lpszAddress, bool bWaitSuccess) {
	if (IsListen())
		return BASIC_NET_ALREADY_LISTEN;
	if (IsToClose())
		return BASIC_NET_TOCLOSE_ERROR;
	uint32_t dwLinkNetStatus = GetSessionStatus(TIL_SS_LINK);
	if(dwLinkNetStatus != TIL_SS_IDLE)
		return BASIC_NET_ALREADY_LISTEN;

	int32_t lReturn = BASIC_NET_OK;
	evutil_socket_t socketfd = INVALID_SOCKET;
	do {
		sockaddr_storage addr;
		int addrlen = sizeof(addr);
		if (evutil_parse_sockaddr_port(lpszAddress, (sockaddr*)&addr, &addrlen) != 0) {
			lReturn = BASIC_NET_ADDRESS_ERROR;
			break;
		}
#ifdef BASICWINDOWS_USE_IOCP
		for(int i = 0; i < DEFAULT_SERVERACCEPTEXDATA_COUNT; i++){
			m_acceptData[i].InitBuffer(addr);
		}
#endif

		socketfd = socket(addr.ss_family, SOCK_STREAM, 0);
		if (socketfd == INVALID_SOCKET) {
			lReturn = BASIC_NET_SOCKET_ERROR;
			break;
		}
		evutil_make_socket_nonblocking(socketfd);
		evutil_make_listen_socket_reuseable(socketfd);
		evutil_make_listen_socket_reuseable_port(socketfd);
		// bind our name to the socket
		int nRet = ::bind(socketfd, (::sockaddr*)&addr, addrlen);   //I know this
		if (nRet != 0) {
			lReturn = BASIC_NET_BIND_ERROR;
			break;
		}
		// Set the socket to listen
		nRet = listen(socketfd, 0x7fffffff);
		if (nRet != 0) {
			lReturn = BASIC_NET_LISTEN_ERROR;
			break;
		}
	} while (0);
	if (lReturn == BASIC_NET_OK) {
		SetSessionStatus(TIL_SS_LISTENING, TIL_SS_LINK);
#ifdef BASICWINDOWS_USE_IOCP
		InitListenEvent(socketfd);
#else
		SetLibEvent([](CBasicNet_Socket* pSession, intptr_t lRevert)->void {
			CBasicNet_SocketListen* pServer = (CBasicNet_SocketListen*)pSession;
			evutil_socket_t socketfd = lRevert;
			pServer->InitListenEvent(lRevert);
		}, socketfd);
		if (bWaitSuccess) {
			while (!IsListen()) {
				m_eventListen.Lock();
			}
		}
#endif
	}
	else if (socketfd != INVALID_SOCKET) {
		evutil_closesocket(socketfd);
	}
	return lReturn;
}

//! ontimer线程
bool CBasicNet_SocketListen::OnTimer(unsigned int nTick) {
	if (CBasicNet_Socket::OnTimer(nTick)) {
		m_pFather->OnTimer(nTick);
		return true;
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////
void OnLinkListenRead(evutil_socket_t fd, short event, void *arg) {
	CBasicNet_SocketListen *pLink = (CBasicNet_SocketListen *)arg;
	if (pLink->m_socketfd == fd) {
		pLink->AcceptClient();
	}
}

void CBasicNet_SocketListen::InitListenEvent(evutil_socket_t socketfd) {
	m_socketfd = socketfd;
#ifdef BASICWINDOWS_USE_IOCP
	CreateIoCompletionPort((HANDLE)m_socketfd, m_pThread->m_hCompletionPort, (DWORD)this, g_nEventThreadCount);
	for(int i = 0; i < DEFAULT_SERVERACCEPTEXDATA_COUNT; i++){
		m_acceptData[i].StartAccept(this);
	}
#else
	event_set(&m_revent, m_socketfd, EV_READ | EV_PERSIST, OnLinkListenRead, this);
	event_base_set(m_pThread->m_base, &m_revent);
	event_add(&m_revent, NULL);
	m_eventListen.SetEvent();
#endif
}

void CBasicNet_SocketListen::AcceptClient() {
	if (IsToClose()) {
		Close();
		return;
	}
	sockaddr_storage addr;
	memset(&addr, 0, sizeof(addr));
	socklen_t addrlen = sizeof(addr);
	evutil_socket_t s = accept(m_socketfd, (::sockaddr*)&addr, &addrlen);   //I know this
	if (s == INVALID_SOCKET) {
		return;
	}
	DoAcceptClient(s, m_pFather->CreateServerClientSession(m_nChildThread++), (sockaddr*)&addr);
}
void CBasicNet_SocketListen::DoAcceptClient(evutil_socket_t s, CBasicSessionNetServerSession* pAcceptSession, sockaddr* pAddr){
	if(m_pPreSend != NULL){
		pAcceptSession->RegistePreSend(m_pPreSend);
	}
	CBasicNet_SocketSession* pTransfer = (CBasicNet_SocketSession*)pAcceptSession->GetBasicNet_Socket();
	pTransfer->Accept(s, pAddr);
	m_pFather->AcceptClient(pAcceptSession);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBasicSessionNetServer::IsListen() {
	return ((CBasicNet_SocketListen*)m_pSocket)->IsListen();
}

int32_t CBasicSessionNetServer::Listen(const char* lpszAddress, bool bWaitSuccess) {
	int32_t nRet = ((CBasicNet_SocketListen*)m_pSocket)->Listen(lpszAddress, bWaitSuccess);
	m_strListenAddr = lpszAddress;	
	return nRet;
}

//获取用户在线数
long CBasicSessionNetServer::GetOnlineSessionCount() {
	basiclib::CSingleLock lock(&m_mtxCSession, TRUE);
	return m_mapClientSession.size();
}

void CBasicSessionNetServer::SendToAll(void * pData, int nLength, DWORD dwFlag, bool bTrasmit){
	VTClientSession vtUser;
	CopyClientSession(vtUser);
	if (bTrasmit) {
		for (auto& client : vtUser) {
			if (client->IsConnected() && client->IsTransmit()) {
				client->Send(pData, nLength, dwFlag);
			}
		}
	}
	else {
		for (auto& client : vtUser) {
			if (client->IsConnected()) {
				client->Send(pData, nLength, dwFlag);
			}
		}
	}
}

//断开所有连接,断开前的回调
void CBasicSessionNetServer::CloseAllSession(const std::function<void(CBasicSessionNetServerSession* pSession)>& func){
	VTClientSession vtUser;
	CopyClientSession(vtUser);
	for (auto& client : vtUser){
		func(client.GetResFunc());
		client->Close();
	}
}
void CBasicSessionNetServer::CloseAll(){
	basiclib::CSingleLock lock(&m_mtxCSession, TRUE);
	for(auto& client : m_mapClientSession){
		client.second->Close();
	}
}
//! 获取状态信息
void CBasicSessionNetServer::GetNetStatus(CBasicString& strStatus){
	VTClientSession vtUser;
	CopyClientSession(vtUser);

	basiclib::CBasicString strVal;
	strVal.Format("监听: %s  连接队列如下：\r\n", m_strListenAddr.c_str());
	for (auto& client : vtUser){
		client->GetNetStatus(strVal);
	}
	strStatus += strVal;
}

CRefBasicSessionNetServerSession CBasicSessionNetServer::GetClientBySessionID(uint32_t nSessionID){
	basiclib::CSingleLock lock(&m_mtxCSession, TRUE);
	MapClientSession::iterator iter = m_mapClientSession.find(nSessionID);
	if (iter != m_mapClientSession.end()){
		return iter->second;
	}
	return nullptr;
}


void CBasicSessionNetServer::CopyClientSession(VTClientSession& vtClient){
	basiclib::CSingleLock lock(&m_mtxCSession, TRUE);
	int nSize = m_mapClientSession.size();
	if (nSize > 0){
		vtClient.reserve(nSize);
		for (auto& client : m_mapClientSession){
			vtClient.push_back(client.second);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBasicSessionNetServerSession* CBasicSessionNetServer::CreateServerClientSession(uint32_t nSessionID) {
	CBasicSessionNetServerSession* pNotify = ConstructSession(nSessionID);
	pNotify->bind_rece(m_funcReceive);
	pNotify->bind_connect(m_funcConnect);
	pNotify->bind_disconnect(MakeFastFunction(this, &CBasicSessionNetServer::OnClientDisconnectCallback));
	pNotify->bind_idle(m_funcIdle);
	pNotify->bind_error(m_funcError);
	return pNotify;
}

CBasicSessionNetServerSession* CBasicSessionNetServer::ConstructSession(uint32_t nSessionID) {
	return CBasicSessionNetServerSession::CreateNetServerSession(nSessionID, m_usRecTimeout);
}

//! 接收对象
void CBasicSessionNetServer::AcceptClient(CBasicSessionNetServerSession* pClient) {
	basiclib::CSingleLock lock(&m_mtxCSession, TRUE);
	m_mapClientSession[pClient->GetSessionID()] = pClient;
}

int32_t CBasicSessionNetServer::OnClientDisconnectCallback(CBasicSessionNetNotify* pClient, uint32_t p2) {
	CBasicSessionNetServerSession* pSession = (CBasicSessionNetServerSession*)pClient;
	//disconnect
	int32_t lRet = m_funcDisconnect ? m_funcDisconnect(pClient, p2) : BASIC_NET_OK;

	//delete
	uint32_t nSessionID = pSession->GetSessionID();
	{
		basiclib::CSingleLock lock(&m_mtxCSession, TRUE);
		m_mapClientSession.erase(nSessionID);
	}
	pSession->SafeDelete();

	return lRet;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
//ontimer线程
void CBasicSessionNetServer::OnTimer(unsigned int nTick) {
	if (nTick % 10 == 0) {
		if (!m_strListenAddr.IsEmpty() && !IsListen()) {
			int32_t lRet = Listen(m_strListenAddr.c_str());
			if (BASIC_NET_OK == lRet) {
				basiclib::BasicLogEventV(DebugLevel_Info, "ListenPort [%s] OK", m_strListenAddr.c_str());
			}
			else if (BASIC_NET_ALREADY_LISTEN != lRet) {
				Close();
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////

__NS_BASIC_END

