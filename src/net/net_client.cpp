#include "../inc/basic.h"
#include "net_client.h"
#include "net_mgr.h"
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
#endif

__NS_BASIC_START

extern int			g_nEventThreadCount;
#ifdef BASICWINDOWS_USE_IOCP
class CBasicNet_SocketClient;
struct WinDNSContextQuery{
	OVERLAPPED				m_QueryOverlapped;
	PADDRINFOEX				m_QueryResults;
	HANDLE					m_CompleteEvent;
	HANDLE					m_CancelHandle;
	CBasicNet_SocketClient* m_pClient;
	timeval					m_timeout;
};
#endif

class CBasicNet_SocketClient : public CBasicNet_SocketTransfer{
public:
	CBasicNet_SocketClient(CBasicSessionNetNotify* pFather, uint32_t nSessionID, uint16_t usRecTimeout = 0);
	virtual ~CBasicNet_SocketClient();

	//! 连接
	int32_t Connect(const char* lpszAddress);

	//! 重连
	int32_t DoConnect();

	//! 获取状态
	virtual void GetNetStatus(CBasicString& strStatus);

	//! 获取连接地址
	basiclib::CBasicString& GetConnectAddr(){ return m_strConnectAddr; }

#ifdef BASICWINDOWS_USE_IOCP
	//! client连接使用
	virtual void ClientConnectEx(){
		//已经绑定到IOCP，开启receive
		if(m_pThread->StartRecvData(this)){
			OnConnect(BASIC_NETCODE_SUCC);
		}
		else{
			OnConnect(BASIC_NETCODE_CLOSE_REMOTE);
		}
	}

	//! dns解析结束
	static friend VOID WINAPI QueryDNSCompleteCallback(_In_ DWORD Error, _In_ DWORD Bytes, _In_ LPOVERLAPPED Overlapped);
#endif
protected:
	//! dns异步解析
	static void DNSParseConnectCallback(int errcode, struct  evutil_addrinfo* addr, void* ptr);

	int32_t RealOnConnect(sockaddr_storage* pAddr, int addrlen);

	//! 初始化事件
	void InitClientEvent(bool bAddWrite = true, bool bAddRead = true);
protected:
	basiclib::CBasicString			m_strConnectAddr;
	bool							m_bIPV6;
	basiclib::CBasicString			m_strParseConnectAddr;
	uint16_t						m_nParseConnectPort;
#ifdef BASICWINDOWS_USE_IOCP
	basiclib::CWBasicString			m_strParseConnectAddrW;
	OVERLAPPEDPLUS					m_olConnectEx;
	WinDNSContextQuery				m_dnsQuery;
	bool							m_bDNS;
#endif
};
#ifdef BASICWINDOWS_USE_IOCP
VOID WINAPI QueryDNSCompleteCallback(_In_ DWORD Error, _In_ DWORD Bytes, _In_ LPOVERLAPPED Overlapped){
	UNREFERENCED_PARAMETER(Bytes);
	WinDNSContextQuery* pQuery = CONTAINING_RECORD(Overlapped, WinDNSContextQuery, m_QueryOverlapped);
	if(Error != ERROR_SUCCESS){
		basiclib::BasicLogEventErrorV("dns parse error(%s:%d)", pQuery->m_pClient->m_strParseConnectAddr.c_str(), Error);
	}
	else{
		sockaddr_storage storageaddr;
		int addrlen = sizeof(sockaddr_storage);
		PADDRINFOEX ai = nullptr;
		struct sockaddr_in* ai_ipv4 = nullptr;
		struct sockaddr_in6* ai_ipv6 = nullptr;
		bool bConnect = false;
		for(ai = pQuery->m_QueryResults; ai; ai = ai->ai_next){
			const char* s = NULL;
			if(ai->ai_family == AF_INET){
				ai_ipv4 = (struct sockaddr_in *)ai->ai_addr;
			}
			else if(ai->ai_family == AF_INET6){
				ai_ipv6 = (struct sockaddr_in6 *)ai->ai_addr;
			}
		}
		if(ai_ipv6 && (pQuery->m_pClient->m_bIPV6 || ai_ipv4 == nullptr)){
			ai_ipv6->sin6_port = htons(pQuery->m_pClient->m_nParseConnectPort);
			memset((struct sockaddr*)&storageaddr, 0, addrlen);
			memcpy((struct sockaddr*)&storageaddr, ai_ipv6, sizeof(sockaddr_in6));
			addrlen = sizeof(sockaddr_in6);
			pQuery->m_pClient->RealOnConnect(&storageaddr, addrlen);
		}
		else if(ai_ipv4){
			ai_ipv4->sin_port = htons(pQuery->m_pClient->m_nParseConnectPort);
			memset((struct sockaddr*)&storageaddr, 0, addrlen);
			memcpy((struct sockaddr*)&storageaddr, ai_ipv4, sizeof(sockaddr_in));
			addrlen = sizeof(sockaddr_in);
			pQuery->m_pClient->RealOnConnect(&storageaddr, addrlen);
		}
		FreeAddrInfoExW(pQuery->m_QueryResults);
		pQuery->m_QueryResults = nullptr;
	}
	//需要先减一次引用
	pQuery->m_pClient->GetRealSessionNet()->DelRef();
	pQuery->m_pClient->m_bDNS = false;
}
#endif
CBasicSessionNetClient* CreateNetWithClient(size_t nClassSize, uint16_t usRecTimeout, const std::function<CBasicSessionNetClient*(void*)>& func){
	void* pData = basiclib::BasicAllocate(nClassSize + sizeof(CBasicNet_SocketClient));
	CBasicSessionNetClient* pClient = func(pData);
	pClient->InitSocket(new ((char*)pData + nClassSize) CBasicNet_SocketClient(pClient, CBasicNet_Socket::GetDefaultCreateSessionID(), usRecTimeout));
	return pClient;
}

CBasicNet_SocketClient::CBasicNet_SocketClient(CBasicSessionNetNotify* pFather, uint32_t nSessionID, uint16_t usRecTimeout) : CBasicNet_SocketTransfer(pFather, nSessionID, usRecTimeout){
#ifdef BASICWINDOWS_USE_IOCP
	m_olConnectEx.m_ioType = IOConnect;
	memset(&m_dnsQuery, 0, sizeof(WinDNSContextQuery));
	m_dnsQuery.m_pClient = this;
	m_dnsQuery.m_timeout.tv_sec = 5;
	m_bDNS = false;
#endif
}
CBasicNet_SocketClient::~CBasicNet_SocketClient(){

}
//连接
int32_t CBasicNet_SocketClient::DoConnect(){
	return RealOnConnect(nullptr, 0);
}

bool ParseAddress(const char *ip_as_string, basiclib::CBasicString& strAddress, uint16_t& nPort, bool& bIPV6){
	/* recognized formats are:
	* [ipv6]:port
	* ipv4:port
	*/
	char buf[128];
	const char* cp = strchr(ip_as_string, ':');
	if(*ip_as_string == '['){
		size_t len;
		if(!(cp = strchr(ip_as_string, ']'))){
			return false;
		}
		len = (cp - (ip_as_string + 1));
		if(len > sizeof(buf) - 1){
			return false;
		}
		memcpy(buf, ip_as_string + 1, len);
		buf[len] = '\0';
		strAddress = buf;
		if(cp[1] == ':')
			nPort = atoi(cp + 2);
		else
			return false;
		bIPV6 = true;
		return true;
	}
	else if(cp){
		if(cp - ip_as_string > (int)sizeof(buf) - 1){
			return false;
		}
		memcpy(buf, ip_as_string, cp - ip_as_string);
		buf[cp - ip_as_string] = '\0';
		strAddress = buf;
		nPort = atoi(cp + 1);
		bIPV6 = false;
		return true;
	}
	return false;
}

int32_t CBasicNet_SocketClient::Connect(const char* lpszAddress){
	if(lpszAddress == nullptr || lpszAddress[0] == '\0'){
		return BASIC_NET_INVALID_ADDRESS;
	}
	if(m_strConnectAddr.CompareNoCase(lpszAddress) != 0){
		m_strConnectAddr = lpszAddress;
		//解析出port和地址
		if(!ParseAddress(m_strConnectAddr.c_str(), m_strParseConnectAddr, m_nParseConnectPort, m_bIPV6)){
			return BASIC_NET_INVALID_ADDRESS;
		}
#ifdef BASICWINDOWS_USE_IOCP
		else{
			m_strParseConnectAddrW = basiclib::Basic_MultiStringToWideString(m_strParseConnectAddr.c_str(), m_strParseConnectAddr.GetLength());
		}
#endif
	}
	return DoConnect();
}

int32_t CBasicNet_SocketClient::RealOnConnect(sockaddr_storage* pAddr, int addrlen){
	uint32_t dwRelease = GetSessionStatus(TIL_SS_RELEASE_MASK);
	uint32_t dwLinkNetStatus = GetSessionStatus(TIL_SS_LINK);
	uint32_t dwCloseNetStatus = GetSessionStatus(TIL_SS_CLOSE);
	if(dwRelease != 0){
		return BASIC_NET_RELEASE_ERROR;
	}
	if(dwCloseNetStatus != TIL_SS_NORMAL){
		int32_t lRet = BASIC_NET_GENERIC_ERROR;
		switch(dwCloseNetStatus){
		case TIL_SS_TOCLOSE:
		{
			lRet = BASIC_NET_TOCLOSE_ERROR;
			break;
		}
		}
		return lRet;
	}
	if(dwLinkNetStatus != TIL_SS_IDLE){
		int32_t lRet = BASIC_NET_GENERIC_ERROR;
		switch(dwLinkNetStatus){
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
	if(m_strConnectAddr.IsEmpty()){
		return BASIC_NET_INVALID_ADDRESS;
	}
	int32_t lReturn = BASIC_NET_OK;
	evutil_socket_t socketfd = INVALID_SOCKET;
	do{
		sockaddr_storage addr;
		if(pAddr == nullptr){
			addrlen = sizeof(addr);
			if(evutil_parse_sockaddr_port(m_strConnectAddr.c_str(), (sockaddr*)&addr, &addrlen) != 0){
#ifdef BASICWINDOWS_USE_IOCP
				if(!m_bDNS){
					m_bDNS = true;
					ADDRINFOEX hints;
					ZeroMemory(&hints, sizeof(hints));
					hints.ai_family = AF_UNSPEC;

					INT nError = GetAddrInfoExW(m_strParseConnectAddrW.c_str(), NULL, NS_DNS, NULL,
												&hints, &m_dnsQuery.m_QueryResults, &m_dnsQuery.m_timeout, &m_dnsQuery.m_QueryOverlapped, QueryDNSCompleteCallback, &m_dnsQuery.m_CancelHandle);
					//dns解析一次需要增加一次引用
					GetRealSessionNet()->AddRef();
					if(nError != WSA_IO_PENDING && nError != NO_ERROR){
						QueryDNSCompleteCallback(nError, 0, &m_dnsQuery.m_QueryOverlapped);
						lReturn = BASIC_NET_GENERIC_ERROR;
					}
				}
#else
				SetLibEvent([](CBasicNet_Socket* pSession, intptr_t lRevert)->void{
					CBasicNet_SocketClient* pClient = (CBasicNet_SocketClient*)pSession;
					if(pClient->m_pThread->DNSParse(pClient->m_strParseConnectAddr.c_str(), CBasicNet_SocketClient::DNSParseConnectCallback, pClient)){
						pClient->GetRealSessionNet()->AddRef();
					}
				});
#endif
			}
			else{
				pAddr = &addr;
			}
		}
		if(pAddr != nullptr){
			evutil_socket_t socketfd = socket(pAddr->ss_family, SOCK_STREAM, 0);
			if(socketfd == INVALID_SOCKET){
				lReturn = BASIC_NET_SOCKET_ERROR;
				break;
			}
			evutil_make_socket_nonblocking(socketfd);
			evutil_make_listen_socket_reuseable(socketfd);
			evutil_make_listen_socket_reuseable_port(socketfd);
#ifdef BASICWINDOWS_USE_IOCP
			struct sockaddr_storage ss;
			memset(&ss, 0, sizeof(ss));
			if(pAddr->ss_family == AF_INET){
				struct sockaddr_in *sin = (struct sockaddr_in *)&ss;
				sin->sin_family = AF_INET;
				sin->sin_addr.s_addr = INADDR_ANY;
			}
			else if(pAddr->ss_family == AF_INET6){
				struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&ss;
				sin6->sin6_family = AF_INET6;
				sin6->sin6_addr = in6addr_any;
			}
			if(::bind(socketfd, (struct sockaddr *)&ss, sizeof(ss)) >= 0 || WSAGetLastError() == WSAEINVAL){
				CreateIoCompletionPort((HANDLE)socketfd, m_pThread->m_hCompletionPort, (DWORD)this, g_nEventThreadCount);

				IOCPExt_Func& func = CNetThread::GetExtFunc();
				BOOL bRet = func.ConnectEx(socketfd, (::sockaddr*)pAddr, addrlen, NULL, 0, NULL, &m_olConnectEx.m_ol);
				int nError = WSAGetLastError();
				if(bRet || nError == ERROR_IO_PENDING){
					SetSessionStatus(TIL_SS_CONNECTING, TIL_SS_LINK);
					m_socketfd = socketfd;
				}
				else{
					lReturn = BASIC_NET_GENERIC_ERROR;
				}
			}
			else{
				lReturn = BASIC_NET_GENERIC_ERROR;
			}
#else
			int nRet = connect(socketfd, (::sockaddr*)pAddr, addrlen);
			if(nRet == 0){
				m_socketfd = socketfd;
				event_set(&m_revent, m_socketfd, EV_READ | EV_PERSIST, OnLinkRead, this);
				event_base_set(m_pThread->m_base, &m_revent);
				event_set(&m_wevent, m_socketfd, EV_WRITE, OnLinkWrite, this);
				event_base_set(m_pThread->m_base, &m_wevent);
				SetLibEvent([](CBasicNet_Socket* pSession, intptr_t lRevert)->void{
					//需要先初始化
					((CBasicNet_SocketClient*)pSession)->InitClientEvent(false);
					((CBasicNet_SocketClient*)pSession)->OnConnect(BASIC_NETCODE_SUCC);
				});
			}
#ifdef __BASICWINDOWS
			else if(errno == EINPROGRESS || WSAGetLastError() == WSAEWOULDBLOCK)
#else
			else if(errno == EINPROGRESS)
#endif
			{
				SetSessionStatus(TIL_SS_CONNECTING, TIL_SS_LINK);
				//wait for write onconnect
				m_socketfd = socketfd;
				event_set(&m_revent, m_socketfd, EV_READ | EV_PERSIST, OnLinkRead, this);
				event_base_set(m_pThread->m_base, &m_revent);
				event_set(&m_wevent, m_socketfd, EV_WRITE, OnLinkWrite, this);
				event_base_set(m_pThread->m_base, &m_wevent);
				SetLibEvent([](CBasicNet_Socket* pSession, intptr_t lRevert)->void{
					((CBasicNet_SocketClient*)pSession)->InitClientEvent(true, false);
				});
			}
			else{
				//int nRetErrorNo = errno;
				lReturn = BASIC_NET_GENERIC_ERROR;
			}
#endif
		}
	} while(0);

	if(lReturn != BASIC_NET_OK && socketfd != INVALID_SOCKET){
		evutil_closesocket(socketfd);
	}
	return lReturn;
}

void CBasicNet_SocketClient::DNSParseConnectCallback(int errcode, struct  evutil_addrinfo* addr, void* ptr){
	CBasicNet_SocketClient* pClient = (CBasicNet_SocketClient*)ptr;
	if(errcode){
		basiclib::BasicLogEventErrorV("dns parse error(%s:%s)", pClient->m_strParseConnectAddr.c_str(), evutil_gai_strerror(errcode));
	}
	else{
		sockaddr_storage storageaddr;
		int addrlen = sizeof(sockaddr_storage);
		struct evutil_addrinfo* ai = nullptr;
		struct sockaddr_in* ai_ipv4 = nullptr;
		struct sockaddr_in6* ai_ipv6 = nullptr;
		bool bConnect = false;
		for(ai = addr; ai; ai = ai->ai_next){
			const char* s = NULL;
			if(ai->ai_family == AF_INET){
				ai_ipv4 = (struct sockaddr_in *)ai->ai_addr;
			}
			else if(ai->ai_family == AF_INET6){
				ai_ipv6 = (struct sockaddr_in6 *)ai->ai_addr;
			}
		}
		if(ai_ipv6 && (pClient->m_bIPV6 || ai_ipv4 == nullptr)){
			ai_ipv6->sin6_port = htons(pClient->m_nParseConnectPort);
			memset((struct sockaddr*)&storageaddr, 0, addrlen);
			memcpy((struct sockaddr*)&storageaddr, ai_ipv6, sizeof(sockaddr_in6));
			addrlen = sizeof(sockaddr_in6);
			pClient->RealOnConnect(&storageaddr, addrlen);
		}
		else if(ai_ipv4){
			ai_ipv4->sin_port = htons(pClient->m_nParseConnectPort);
			memset((struct sockaddr*)&storageaddr, 0, addrlen);
			memcpy((struct sockaddr*)&storageaddr, ai_ipv4, sizeof(sockaddr_in));
			addrlen = sizeof(sockaddr_in);
			pClient->RealOnConnect(&storageaddr, addrlen);
		}
		evutil_freeaddrinfo(addr);
	}
	//! 必须先减少一次引用,在DNSparse里面会增加一次引用
	pClient->GetRealSessionNet()->DelRef();
}

//! 获取状态信息
void CBasicNet_SocketClient::GetNetStatus(CBasicString& strStatus){
	uint32_t dwLinkNetStatus = GetSessionStatus(TIL_SS_LINK);
	uint32_t dwCloseNetStatus = GetSessionStatus(TIL_SS_CLOSE);
	char szBuf[32] = { 0 };
	if(dwCloseNetStatus != TIL_SS_NORMAL){
		switch(dwCloseNetStatus){
		case TIL_SS_TOCLOSE:
		{
			strcpy(szBuf, "TOCLOSE");
			break;
		}
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
			break;
		}
		default:
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
				   m_strConnectAddr.c_str(), szBufTime, m_usRecTimeout, szBuf, m_stNet.m_dwReceBytes / 1024, m_stNet.m_dwSendBytes / 1024, dRecvRate, dSendRate);
	strStatus += strTemp;
}


void CBasicNet_SocketClient::InitClientEvent(bool bAddWrite, bool bAddRead){
#ifndef BASICWINDOWS_USE_IOCP
	//read data
	if(bAddRead){
		event_add(&m_revent, NULL);
	}
	if(bAddWrite){
		//write data
		event_add(&m_wevent, NULL);
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int32_t CBasicSessionNetClient::DoConnect(){
	return ((CBasicNet_SocketClient*)m_pSocket)->DoConnect();
}

int32_t CBasicSessionNetClient::Connect(const char* lpszAddress){
	return ((CBasicNet_SocketClient*)m_pSocket)->Connect(lpszAddress);
}

//! 获取连接地址
basiclib::CBasicString& CBasicSessionNetClient::GetConnectAddr(){
	return ((CBasicNet_SocketClient*)m_pSocket)->GetConnectAddr();
}


__NS_BASIC_END

