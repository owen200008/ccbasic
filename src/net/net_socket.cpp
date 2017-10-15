#include "../inc/basic.h"
#include "net_mgr.h"
#include "net.h"

__NS_BASIC_START

///////////////////////////////////////////////////////////////////////////////
extern CBasicNetMgv* m_gNetMgrPoint;
extern CNetThread*	g_pEventThreads;
extern int			g_nEventThreadCount;
extern int			g_nIntThreadCount;
///////////////////////////////////////////////////////////////////////////////
std::atomic<uint32_t> CBasicNet_Socket::m_defaultCreateSession(0);
CBasicNet_Socket::CBasicNet_Socket(uint32_t nSessionID){
	//第一次不能用m_gNetMgrPoint，需要保证构造出对象
	CBasicSingletonNetMgv::Instance().AddToTimer(this);
	m_pThread = &g_pEventThreads[nSessionID % g_nEventThreadCount];
	m_socketfd = INVALID_SOCKET;
	m_unSessionStatus = 0;
	m_pPreSend = nullptr;
}
CBasicNet_Socket::~CBasicNet_Socket(){
#ifdef _DEBUG
	if(m_socketfd != INVALID_SOCKET){
		ASSERT(0);
	}
#endif
	if(m_pPreSend != NULL){
		delete m_pPreSend;
	}
}

int CBasicNet_Socket::RegistePreSend(CBasicPreSend* pFilter, uint32_t dwRegOptions){
	if(pFilter != NULL){
		if(m_pPreSend != NULL){
			delete m_pPreSend;
			m_pPreSend = NULL;
		}
		if(m_pPreSend == NULL){
			m_pPreSend = pFilter->Construct();
		}
	}
	return 1;
}

//! 关闭
void CBasicNet_Socket::Close(bool bRemote, bool bMustClose){
	if(m_socketfd == INVALID_SOCKET)
		return;
	SetToClose();

	uint32_t nSetRevert = 0;
	if(bRemote)
		nSetRevert |= 0x00000001;
	if(bMustClose)
		nSetRevert |= 0x00000002;
	//may be loop, take care
	SetLibEvent([](CBasicNet_Socket* pSession, intptr_t lRevert)->void{
		uint32_t nSetRevert = lRevert;
		if(nSetRevert != 0 || pSession->CanClose()){
			BOOL bRemote = (nSetRevert & 0x00000001) != 0;
			pSession->CloseCallback(bRemote);
		}
	}, nSetRevert);
}

//! 回调中可以安全删除
void CBasicNet_Socket::SafeDelete(){
	SetToSafeDelete();
	SetLibEvent([](CBasicNet_Socket* pSession, intptr_t lRevert)->void{
		if(pSession->GetSocketID() != INVALID_SOCKET){
			pSession->CloseCallback(FALSE);
		}
		//从ontimer删除
		m_gNetMgrPoint->DelToTimer(pSession);
	});
}

//! 判断是否可以关闭
bool CBasicNet_Socket::CanClose(){
	return GetSessionStatus(TIL_SS_LINK) != TIL_SS_CONNECTING;
}
/////////////////////////////////////////////////////////////////////////////////////
bool CBasicNet_Socket::IsConnected(){
	return GetSessionStatus(TIL_SS_LINK) == TIL_SS_CONNECTED;
}
//! 是否认证成功
bool CBasicNet_Socket::IsTransmit(){
	return GetSessionStatus(TIL_SS_SHAKEHANDLE_MASK) == TIL_SS_SHAKEHANDLE_TRANSMIT;
}

//! 判断是否准备close
bool CBasicNet_Socket::IsToClose(){
	return GetSessionStatus(TIL_SS_CLOSE) >= TIL_SS_TOCLOSE;
}

//! 设置准备关闭
void CBasicNet_Socket::SetToClose(){
	SetSessionStatus(TIL_SS_TOCLOSE, TIL_SS_CLOSE);
}
void CBasicNet_Socket::SetToSafeDelete(){
	SetSessionStatus(TIL_SS_TOSAFEDELETE, TIL_SS_RELEASE_MASK);
}
/////////////////////////////////////////////////////////////////////////////////////
//! ontimer线程
bool CBasicNet_Socket::OnTimer(unsigned int nTick){
	if(GetSessionStatus(TIL_SS_CLOSE) == TIL_SS_TOCLOSE){
		Close();
		return false;
	}
	return GetSessionStatus(TIL_SS_RELEASE_MASK) == 0;
}
/////////////////////////////////////////////////////////////////////////////////////
//! 加入队列
void CBasicNet_Socket::AddSocketCallFunc(CEventQueueItem* pItem){
	long lLength = 0;
	{
		basiclib::CSpinLockFuncNoSameThreadSafe lock(&m_lockMsg, TRUE);
		lLength = m_smBuf.GetDataLength();
		m_smBuf.AppendDataEx((char*)pItem, sizeof(CEventQueueItem));
	}
	if(lLength == 0){
		m_pThread->AddMessageQueue(this);
	}
}

void CBasicNet_Socket::RunSocketCallFunc(){
	{
		basiclib::CSpinLockFuncNoSameThreadSafe lock(&m_lockMsg, TRUE);
		swap(m_smIOCPBuf, m_smBuf);
	}
	int nCount = m_smIOCPBuf.GetDataLength() / sizeof(CEventQueueItem);
	if(nCount > 0){
		char* pBegin = m_smIOCPBuf.GetDataBuffer();
		for(int i = 0; i < nCount; i++){
			CEventQueueItem* pItem = (CEventQueueItem*)(pBegin + i * sizeof(CEventQueueItem));
			pItem->m_pCallFunc(pItem->m_pRefNetSession, pItem->m_lRevert);
		}
		m_smIOCPBuf.SetDataLength(0);
	}
}

void CBasicNet_Socket::SetLibEvent(pCallSameRefNetSessionFunc pCallback, intptr_t lRevert){
	m_pThread->SetEvent(this, pCallback, lRevert);
}

void CBasicNet_Socket::InitMember(){
	m_socketfd = INVALID_SOCKET;
	m_unSessionStatus &= TIL_RESET_MASK;
}

void CBasicNet_Socket::CloseCallback(BOOL bRemote, DWORD dwNetCode){
	if(m_socketfd != INVALID_SOCKET){
#ifdef BASICWINDOWS_USE_IOCP
		LINGER lingerStruct;
		lingerStruct.l_onoff = 1;
		lingerStruct.l_linger = 0;
		setsockopt(m_socketfd, SOL_SOCKET, SO_LINGER, (char *)&lingerStruct, sizeof(lingerStruct));   //I know this
		CancelIo((HANDLE)m_socketfd);      //I know this
		shutdown(m_socketfd, SD_SEND);     //I know this
#else
		event_del(&m_revent);
#endif
		evutil_closesocket(m_socketfd);
		InitMember();
	}
}
///////////////////////////////////////////////////////////////////////////////
void OnLinkRead(evutil_socket_t fd, short event, void *arg){
	CBasicNet_SocketTransfer* pLink = (CBasicNet_SocketTransfer*)arg;
	if(pLink != nullptr && pLink->GetSocketID() == fd){
		pLink->OnReadEvent();
	}
}

void OnLinkWrite(evutil_socket_t fd, short event, void *arg){
	CBasicNet_SocketTransfer* pLink = (CBasicNet_SocketTransfer*)arg;
	if(pLink != nullptr && pLink->GetSocketID() == fd){
		pLink->OnWriteEvent();
	}
}

unsigned short CBasicNet_SocketTransfer::m_usTimeoutShakeHandle = 10;
CBasicNet_SocketTransfer::CBasicNet_SocketTransfer(CBasicSessionNetNotify* pFather, uint32_t nSessionID, uint16_t usRecTimeout) : CBasicNet_Socket(nSessionID){
	m_pFather = pFather;
	m_usRecTimeout = usRecTimeout;
	m_unIdleCount = 0;
#ifdef BASICWINDOWS_USE_IOCP
	m_olRead.m_ioType = IORead;
	m_olWrite.m_ioType = IOWrite;
	m_wsaInBuffer.buf = m_byInBuffer;
	m_wsaInBuffer.len = sizeof(m_byInBuffer);
	m_wsaOutBuffer.buf = m_byOutBuffer;
	m_wsaOutBuffer.len = 0;
	m_bSend = false;
#endif
}
CBasicNet_SocketTransfer::~CBasicNet_SocketTransfer(){

}
/////////////////////////////////////////////////////////////////////////////////////
//! ontimer线程
bool CBasicNet_SocketTransfer::OnTimer(unsigned int nTick){
	if(CBasicNet_Socket::OnTimer(nTick)){
		if(IsConnected()){
			//每次发送会重置idlecount
			OnIdle();
			//10s检查超时
			if(m_usRecTimeout != 0 && nTick % 10 == 9){
				time_t tmNow = time(NULL);
				if(IsRecTimeout(tmNow, m_usRecTimeout)){
					Close();
				}
			}
		}
		return true;
	}
	return false;
}

void CBasicNet_SocketTransfer::OnIdle(){
	m_unIdleCount++;
	if(GetSessionStatus(TIL_SS_SHAKEHANDLE_MASK) == TIL_SS_SHAKEHANDLE_TRANSMIT){
		m_pFather->OnIdle(m_unIdleCount);
	}
	else if(m_unIdleCount > m_usTimeoutShakeHandle){
		Close();
		return;
	}
}

/////////////////////////////////////////////////////////////////////////////////////
//! 重新初始化成员
void CBasicNet_SocketTransfer::InitMember(){
	CBasicNet_Socket::InitMember();
	m_unIdleCount = 0;
	m_msgQueue.SetDataLength(0);
#ifdef BASICWINDOWS_USE_IOCP
	m_bSend = false;
	m_wsaOutBuffer.len = 0;
#endif
}

//! 线程内执行函数
void CBasicNet_SocketTransfer::CloseCallback(BOOL bRemote, DWORD dwNetCode){
	if(m_socketfd != INVALID_SOCKET){
		if(bRemote){
			dwNetCode |= BASIC_NETCODE_CLOSE_REMOTE;
		}
#ifndef BASICWINDOWS_USE_IOCP
		event_del(&m_wevent);
#endif
		CBasicNet_Socket::CloseCallback(bRemote);

		OnDisconnect(dwNetCode);
	}
}

//! 断开消息
void CBasicNet_SocketTransfer::OnDisconnect(uint32_t dwNetCode){
	m_pFather->OnDisconnect(dwNetCode);
}
void CBasicNet_SocketTransfer::OnSendData(uint32_t dwIoSize){
	if(IsToClose() || !IsConnected())
		return;
	m_stNet.OnSendData(dwIoSize);
	//每次发送重置idle
	m_unIdleCount = 0;
}

void CBasicNet_SocketTransfer::OnConnect(uint32_t dwNetCode){
	if(dwNetCode & BASIC_NETCODE_SUCC){
		//设置最后收到的时间
		SetSessionStatus(TIL_SS_CONNECTED, TIL_SS_LINK);
		m_stNet.Empty();
		m_stNet.OnReceiveData(0);
	}
	else{
		CloseCallback(TRUE, BASIC_NETCODE_CONNET_FAIL);
		return;
	}
	ResetPreSend();
	int32_t lRet = m_pFather->OnConnect(dwNetCode);
	//如果函数返回 BASIC_NET_HC_RET_HANDSHAKE，表示该连接需要进行握手。否则认为握手成功。
	if(lRet == BASIC_NET_OK){
		SetSessionStatus(TIL_SS_SHAKEHANDLE_TRANSMIT, TIL_SS_SHAKEHANDLE_MASK);
	}
	else if(lRet == BASIC_NET_GENERIC_ERROR){
		Close();
	}
}

uint32_t CBasicNet_SocketTransfer::OnReceive(uint32_t dwNetCode, const char *pszData, int32_t cbData){
	int32_t lRet = m_pFather->OnReceive(dwNetCode, pszData, cbData);
	//返回 BASIC_NET_HR_RET_HANDSHAKE，表示握手成功。对于需要握手的连接，一定要在接收数据的处理函数里面返回这个值。
	if((lRet & BASIC_NET_HR_RET_HANDSHAKE) && !(lRet & NET_ERROR)){
		SetSessionStatus(TIL_SS_SHAKEHANDLE_TRANSMIT, TIL_SS_SHAKEHANDLE_MASK);
	}
	return lRet;
}
void CBasicNet_SocketTransfer::OnError(uint32_t dwNetCode, int32_t lRetCode){
	m_pFather->OnError(dwNetCode, lRetCode);
}


void CBasicNet_SocketTransfer::OnReadEvent(void){
#ifdef BASICWINDOWS_USE_IOCP
	ASSERT(0);
#else
	char szBuf[READBUFFERSIZE_MSG];
	int nReceived = 0;
	while((nReceived = recv(m_socketfd, szBuf, READBUFFERSIZE_MSG, 0)) > 0){
		OnReceiveData(szBuf, nReceived);
	}
	if(nReceived == 0){
		Close(TRUE);
		return;
	}
	else if(nReceived < 0){
		int nNumber = errno;
		if(nNumber == EAGAIN || nNumber == EINTR || errno == EWOULDBLOCK){
			return;
		}
		else{
			BOOL bError = FALSE;
#ifdef __BASICWINDOWS
			if(nReceived == SOCKET_ERROR){
				DWORD dwLastError = WSAGetLastError();   //I know this
				if(dwLastError == WSAEWOULDBLOCK){
					return;
				}
				else if(dwLastError != WSA_IO_PENDING){
					bError = TRUE;
				}
			}
#else
			bError = TRUE;
#endif
			if(bError){
				Close(TRUE);
				return;
			}
		}
	}
#endif
}

void CBasicNet_SocketTransfer::OnReceiveData(const char* pszData, uint32_t dwIoSize){
	int nReceived = (int)dwIoSize;
	int32_t lRet = 0;
	m_stNet.OnReceiveData(nReceived);
	if(m_pPreSend != NULL){
		PreReceiveData(BASIC_NETCODE_SUCC, pszData, nReceived);
		return;
	}
	OnReceive(BASIC_NETCODE_SUCC, pszData, nReceived);
}

void CBasicNet_SocketTransfer::PreReceiveData(uint32_t dwNetCode, const char *pszData, int32_t cbData){
	const char* pPack = pszData;
	long lPackLen = cbData;
	int rRet = PACK_FILTER_SEARCH;
	m_bufCacheTmp.SetDataLength(0);
	rRet = m_pPreSend->OnPreReceive(pPack, lPackLen, m_bufCacheTmp, m_pFather);
	while(rRet == PACK_FILTER_NEXT){
		//同一次接收，响应多次消息
		int32_t lRecRet = OnReceive(dwNetCode | BASIC_NETCODE_FILTER_HANDLE, m_bufCacheTmp.GetDataBuffer(), m_bufCacheTmp.GetDataLength());
		if(lRecRet < BASIC_NET_OK)	//收到错误包后可能被断开
			return;
		m_bufCacheTmp.SetDataLength(0);
		rRet = m_pPreSend->OnPreReceive(NULL, 0, m_bufCacheTmp, m_pFather);
	}
	if(rRet == PACK_FILTER_SKIP){
		OnReceive(dwNetCode, pPack, lPackLen);
		return;
	}
	else if(rRet == PACK_FILTER_HANDLED){
		//do nothing
	}
	else if(rRet == PACK_FILTER_SEARCH){
		pPack = m_bufCacheTmp.GetDataBuffer(lPackLen);
		dwNetCode |= BASIC_NETCODE_FILTER_HANDLE;
		OnReceive(dwNetCode, pPack, lPackLen);
		return;
	}
	else{
		OnError(dwNetCode | BASIC_NETCODE_FILTER_ERROR, BASIC_NET_FILTER_READ_FAIL);
		return;
	}
	return;
}

void CBasicNet_SocketTransfer::OnWriteEvent(){
#ifdef BASICWINDOWS_USE_IOCP
	ASSERT(0);
#else
	uint32_t dwStatus = GetSessionStatus(TIL_SS_LINK);
	if(dwStatus == TIL_SS_CONNECTING){
		int nErr = 0;
		socklen_t nLen = sizeof(nErr);
		getsockopt(m_socketfd, SOL_SOCKET, SO_ERROR, (char *)&nErr, &nLen);
		uint32_t dwNetCode = 0;
		if(nErr == 0){
			dwNetCode = BASIC_NETCODE_SUCC;
			event_add(&m_revent, NULL);
		}
		OnConnect(dwNetCode);
	}
	else{
		SendDataFromQueue();
	}
#endif
}

#ifdef BASICWINDOWS_USE_IOCP
//! IOCP回调发送成功字节
void CBasicNet_SocketTransfer::SendDataSuccessAndCheckSend(DWORD dwIoSize){
	if(dwIoSize > 0){
		OnSendData(dwIoSize);
		m_msgQueue.SendBuffer(dwIoSize);
	}

	int nSendLength = m_msgQueue.GetDataLength();
	if(nSendLength > 4096){
		nSendLength = 4096;
	}
	if(nSendLength > 0){
		m_bSend = true;
		m_wsaOutBuffer.len = nSendLength;
		memcpy(m_wsaOutBuffer.buf, m_msgQueue.GetDataBuffer(), nSendLength);
		int nRetVal = WSASend(m_socketfd, &m_wsaOutBuffer, 1, &m_wsaOutBuffer.len, 0, &m_olWrite.m_ol, NULL);   //I know this
		if(nRetVal == SOCKET_ERROR){
			DWORD dwLastError = WSAGetLastError();   //I know this
			if(dwLastError != WSA_IO_PENDING){
				Close(TRUE);
			}
		}
	}
	else{
		m_bSend = false;
	}
}
#endif
//保证是libevent线程处理
void CBasicNet_SocketTransfer::SendDataFromQueue(){
	if(!IsConnected()){
		return;
	}
#ifdef BASICWINDOWS_USE_IOCP
	if(!m_bSend){
		SendDataSuccessAndCheckSend(0);
	}
#else
	BOOL bError = FALSE;
	int nTotalSend = 0;
	int32_t lSend = 0;
	while(m_msgQueue.SendBuffer(lSend) > 0){
		int nSendLength = m_msgQueue.GetDataLength();
		if(nSendLength > 4096){
			nSendLength = 4096;
		}
		lSend = send(m_socketfd, m_msgQueue.GetDataBuffer(), nSendLength, 0);
		if(lSend >= 0){
			nTotalSend += lSend;
		}
		else{
			int nNumber = errno;
			if(nNumber == EAGAIN){
				event_add(&m_wevent, NULL);
				break;
			}
			else if(nNumber == EINTR){
				lSend = 0;
				continue;
			}
			else{
#ifdef __BASICWINDOWS
				if(lSend == SOCKET_ERROR){
					DWORD dwLastError = WSAGetLastError();   //I know this
					if(dwLastError == WSAEWOULDBLOCK){
						event_add(&m_wevent, NULL);
					}
					else if(dwLastError != WSA_IO_PENDING){
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
	if(bError){
		//有错误
		Close(TRUE);
	}
	else{
		OnSendData(nTotalSend);
	}
#endif
}


void CBasicNet_SocketTransfer::AddSendQueue(SendBufferCache* pSendCache){
	m_msgQueue.AppendDataEx(pSendCache->m_cRevertData, pSendCache->m_cbData);
	SendBufferCache::ReleaseCache(pSendCache);
	SendDataFromQueue();
}

/////////////////////////////////////////////////////////////////////////////////////
bool CBasicNet_SocketTransfer::CanClose(){
	BOOL bRet = CBasicNet_Socket::CanClose();
	if(bRet){
		if(m_msgQueue.GetDataLength() == 0)
			return true;
		return !IsConnected();
	}
	return false;
}

int32_t CBasicNet_SocketTransfer::SendData(SendBufferCacheMgr& sendData){
	SendBufferCache* pCache = sendData.SwapCache();
	SetLibEvent([](CBasicNet_Socket* pSession, intptr_t lRevert)->void{
		CBasicNet_SocketTransfer* pClientSession = (CBasicNet_SocketTransfer*)pSession;
		pClientSession->AddSendQueue((SendBufferCache*)lRevert);
	}, (intptr_t)pCache);
	return pCache->m_cbData;
}

BOOL CBasicNet_SocketTransfer::IsRecTimeout(time_t tmNow, uint16_t nTimeoutSecond){
	if(tmNow - m_stNet.m_tmLastRecTime >= nTimeoutSecond && nTimeoutSecond != 0)
		return TRUE;
	return FALSE;
}

int32_t CBasicNet_SocketTransfer::Send(void *pData, int32_t cbData, uint32_t dwFlag){
	if(IsToClose()){
		return BASIC_NET_TOCLOSE_ERROR;
	}
	if(!IsConnected()){
		return BASIC_NET_NO_CONNECT;
	}
	SendBufferCacheMgr cacheMgr;
	if(m_pPreSend != NULL){
		char* pSendData = (char*)pData;
		long nSendData = cbData;

		int rRet = PACK_FILTER_SEARCH;
		rRet = m_pPreSend->OnPreSend(pSendData, nSendData, dwFlag, cacheMgr);
		if(rRet == PACK_FILTER_SKIP){
			cacheMgr.Reset((const char*)pData, cbData);
		}
		else if(rRet == PACK_FILTER_HANDLED || rRet == PACK_FILTER_SEARCH){
		}
		else{
			OnError(BASIC_NETCODE_FILTER_ERROR, BASIC_NET_FILTER_WRITE_FAIL);
			return BASIC_NET_FILTER_WRITE_FAIL;
		}
		return SendData(cacheMgr);
	}
	else{
		cacheMgr.Reset((const char*)pData, cbData);
		return SendData(cacheMgr);
	}
}

void CBasicNet_SocketTransfer::ResetPreSend(){
	if(m_pPreSend != NULL){
		m_pPreSend->ResetPreSend();
	}
}

__NS_BASIC_END

