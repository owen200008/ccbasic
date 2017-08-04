#include "servertemplate.h"

CNetServerControlSession::CNetServerControlSession(){
    m_bVerify = false;
}

CNetServerControlSession::~CNetServerControlSession()
{

}
void CNetServerControlSession::SuccessLogin(){
	m_bVerify = true;
	m_server->SuccessLogin(this);
}

uint32_t CNetServerControlSession::OnConnect(uint32_t dwNetCode){
	if (!m_server->m_ipTrust.IsIpTrust(GetNetAddress())){
		basiclib::BasicLogEventErrorV("地址：%s 不在白名单内!", GetNetAddress());
		return BASIC_NET_GENERIC_ERROR;
	}
	if (m_server->m_nSessionMaxCount >= 0){
		if (m_server->GetOnlineSessionCount() > m_server->m_nSessionMaxCount){
            basiclib::BasicLogEventErrorV("地址：%s 超过最大连接数 %d!", GetNetAddress(), m_server->m_nSessionMaxCount);
			return BASIC_NET_GENERIC_ERROR;
		}
	}
	//需要认证
    int32_t nRet = basiclib::CBasicSessionNetServerSession::OnConnect(dwNetCode);
    if (nRet == BASIC_NET_OK){
        SuccessLogin();
    }
    return nRet;
}
//////////////////////////////////////////////////////////////////////////////////////
CNetServerControl::CNetServerControl() {
	m_nSessionMaxCount = -1;
	m_handleVerifySuccess = nullptr;
}

CNetServerControl::~CNetServerControl(){
}

void CNetServerControl::SetIpTrust(const char* lpszIpTrust){
	m_ipTrust.SetIPRuler(lpszIpTrust);
}

void CNetServerControl::SetSessionMaxCount(int nCount){
	m_nSessionMaxCount = nCount;
}

int32_t CNetServerControl::StartServer(const char* lpszAddress, basiclib::CBasicPreSend* pPreSend){
	bind_connect(MakeFastFunction(this, &CNetServerControl::OnUserConnect));
	if (pPreSend){
		RegistePreSend(pPreSend);
	}
	int32_t nRet = Listen(lpszAddress, true);
	if (nRet == BASIC_NET_OK){
        basiclib::BasicLogEventV(basiclib::DebugLevel_Info, "ListenPort: %s Success!", lpszAddress);
	}
	else{
		Close();
        basiclib::BasicLogEventV(basiclib::DebugLevel_Info, "ListenPort: %s Error!", lpszAddress);
	}
	return nRet;
}

int32_t CNetServerControl::OnUserConnect(basiclib::CBasicSessionNetNotify* pNotify, Net_UInt dwNetCode){
	return BASIC_NET_OK;
}

//用户登录成功
bool CNetServerControl::SuccessLogin(CNetServerControlSession* pNotify){
	if (m_handleVerifySuccess)
		if (!m_handleVerifySuccess(pNotify)){
			pNotify->Close();
			return false;
		}
	return true;
}

basiclib::CBasicSessionNetServerSession* CNetServerControl::ConstructSession(uint32_t nSessionID){
	return CNetServerControlSession::CreateNetServerSessionWithServer(nSessionID, m_usRecTimeout, this);
}

