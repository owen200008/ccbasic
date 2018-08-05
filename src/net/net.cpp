#include "../inc/basic.h"
#include "net.h"
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
///////////////////////////////////////////////////////////////////////////////////////////////////
extern pGetConfFunc g_GetParamFunc;
void SetNetInitializeGetParamFunc(pGetConfFunc func){
    g_GetParamFunc = func;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
BasicNetStat::BasicNetStat(){
    Empty();
}
void BasicNetStat::Empty(){
    memset(this, 0, sizeof(BasicNetStat));
}
void BasicNetStat::OnSendData(int nSend){
    if(nSend > 0){
        m_dwSendBytes += nSend;
        m_dwSendTimes++;
    }
}
void BasicNetStat::OnReceiveData(int nRece){
    if(nRece > 0){
        m_dwReceBytes += nRece;
        m_dwReceTimes++;
    }
    m_tmLastRecTime = time(NULL);
}
void BasicNetStat::GetTransRate(BasicNetStat& lastData, double& dSend, double& dRecv){
    DWORD tNow = basiclib::BasicGetTickTime();
    if(m_tLastStat > 0 && (tNow - m_tLastStat)  < 10000){
        dSend = m_fLastSendRate;
        dRecv = m_fLastRecvRate;
        return;
    }

    if(m_tLastStat == 0){
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
///////////////////////////////////////////////////////////////////////////////////////////////////
//! 智能指针删除的方式
void CBasicSessionNet::DeleteRetPtrObject(){
    //析构
    this->~CBasicSessionNet();
    m_pSocket->~CBasicNet_Socket();
    basiclib::BasicDeallocate(this);
}


CBasicSessionNet::CBasicSessionNet(){
    m_self = this;
}

CBasicSessionNet::~CBasicSessionNet(){
#ifdef _DEBUG
    if(m_self != nullptr){
        ASSERT(0);
    }
#endif
}

//! 绑定socket
void CBasicSessionNet::InitSocket(CBasicNet_Socket* pSocket){
    m_pSocket = pSocket;
}

void CBasicSessionNet::Close(bool bNoWaitMustClose){
    m_pSocket->Close(false, bNoWaitMustClose);
}

int CBasicSessionNet::RegistePreSend(CBasicPreSend* pFilter, uint32_t dwRegOptions){
    return m_pSocket->RegistePreSend(pFilter, dwRegOptions);
}

//! 获取注册的过滤器
CBasicPreSend* CBasicSessionNet::GetPreSend(){
    return m_pSocket->GetPreSend();
}

//提供安全删除的回调接口
void CBasicSessionNet::SafeDelete(){
    if(m_self == nullptr)
        return;
    m_pSocket->SafeDelete();
    m_self = nullptr;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBasicSessionNetNotify::CBasicSessionNetNotify(){
}
CBasicSessionNetNotify::~CBasicSessionNetNotify(){
}

//! 是否连接
bool CBasicSessionNetNotify::IsConnected(){
    return m_pSocket->IsConnected();
}
//! 是否认证成功
bool CBasicSessionNetNotify::IsTransmit(){
    return m_pSocket->IsTransmit();
}

int32_t CBasicSessionNetNotify::Send(void *pData, int32_t cbData, uint32_t dwFlag){
    if(cbData <= 0)
        return 0;
    return ((CBasicNet_SocketTransfer*)m_pSocket)->Send(pData, cbData, dwFlag);
}

int32_t CBasicSessionNetNotify::Send(basiclib::CBasicSmartBuffer& smBuf, uint32_t dwFlag){
    return Send(smBuf.GetDataBuffer(), smBuf.GetDataLength(), dwFlag);
}

//! 获取网络状态
void CBasicSessionNetNotify::GetNetStatus(CBasicString& strStatus){
    ((CBasicNet_SocketTransfer*)m_pSocket)->GetNetStatus(strStatus);
}

//! 获取网络状态
void CBasicSessionNetNotify::GetNetStatInfo(BasicNetStat& netState){
    ((CBasicNet_SocketTransfer*)m_pSocket)->GetNetStatInfo(netState);
}

__NS_BASIC_END

