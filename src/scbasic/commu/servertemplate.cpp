#include "servertemplate.h"

CNetServerControlClient::CNetServerControlClient(uint32_t nSessionID, CRefNetServerControl pServer) : basiclib::CBasicSessionNetClient(nSessionID, false)
{
	m_server = pServer;
}
CNetServerControlClient::~CNetServerControlClient()
{

}

int32_t CNetServerControlClient::OnConnect(uint32_t dwNetCode)
{
	if (!m_server->m_ipTrust.IsIpTrust(m_szPeerAddr))
	{
		basiclib::BasicLogEventErrorV("地址：%s 不在白名单内!", m_szPeerAddr);
		Close();
		return BASIC_NET_OK;
	}

	if (m_server->m_nSessionMaxCount >= 0)
	{
		if (m_server->GetOnlineSessionCount() > m_server->m_nSessionMaxCount)
		{
            basiclib::BasicLogEventErrorV("地址：%s 超过最大连接数 %d!", m_szPeerAddr, m_server->m_nSessionMaxCount);
			Close();
			return BASIC_NET_OK;
		}
	}
	//需要认证
	return basiclib::CBasicSessionNetClient::OnConnect(dwNetCode);
}
int32_t CNetServerControlClient::OnDisconnect(uint32_t dwNetCode)
{
	return basiclib::CBasicSessionNetClient::OnDisconnect(dwNetCode);
}
int32_t CNetServerControlClient::OnReceive(uint32_t dwNetCode, const char *pszData, int32_t cbData)
{
	return basiclib::CBasicSessionNetClient::OnReceive(dwNetCode, pszData, cbData);
}
//////////////////////////////////////////////////////////////////////////////////////
CNetServerControl::CNetServerControl(uint32_t nSessionID) : basiclib::CBasicSessionNetServer(nSessionID)
{
	m_nSessionMaxCount = -1;
	m_handleVerifySuccess = nullptr;
}

CNetServerControl::~CNetServerControl()
{

}

void CNetServerControl::SetIpTrust(const char* lpszIpTrust)
{
	m_ipTrust.SetIPRuler(lpszIpTrust);
}
void CNetServerControl::SetSessionMaxCount(int nCount)
{
	m_nSessionMaxCount = nCount;
}

void CNetServerControl::OnTimer(uint32_t nTick)
{
	if (nTick % 10 == 1)
	{
		IsListen();
	}
	basiclib::CBasicSessionNetServer::OnTimer(nTick);
}

BOOL CNetServerControl::IsListen()
{
	BOOL bRet = FALSE;
	if (m_strListenAddr.IsEmpty())
	{
		long lRet = Listen(m_strListenAddr.c_str());

		if (BASIC_NET_OK == lRet)
		{
            basiclib::BasicLogEventV(basiclib::DebugLevel_Info, "ListenPort: [%s] Success!", m_strListenAddr.c_str());
			bRet = TRUE;
		}
		else if (BASIC_NET_ALREADY_LISTEN != lRet)
		{
			Close();
		}
		else if (BASIC_NET_ALREADY_LISTEN == lRet)
		{
			bRet = TRUE;
		}
	}
	return bRet;
}

int32_t CNetServerControl::StartServer(const char* lpszAddress, basiclib::CBasicPreSend* pPreSend)
{
	if (pPreSend)
	{
		RegistePreSend(pPreSend);
	}
	int32_t nRet = Listen(lpszAddress, true);
	if (nRet == BASIC_NET_OK)
	{
        basiclib::BasicLogEventV(basiclib::DebugLevel_Info, "ListenPort: %s Success!", lpszAddress);
	}
	else
	{
		Close();
        basiclib::BasicLogEventV(basiclib::DebugLevel_Info, "ListenPort: %s Error!", lpszAddress);
	}
	return nRet;
}

int32_t CNetServerControl::OnUserVerify(basiclib::CBasicSessionNetClient* pNotify, uint32_t dwNetCode, int32_t cbData, const char *pszData)
{
	//先不需要回调上层
	SuccessLogin(pNotify);
	return BASIC_NET_OK;
}

int32_t CNetServerControl::OnVerifyDisconnectCallback(basiclib::CBasicSessionNetClient* pClient, uint32_t p2)
{
	basiclib::CSingleLock lockUser(&m_mtxCSessionVerify, TRUE);
	m_mapClientSessionVerify.erase(pClient->GetSessionID());
	lockUser.Unlock();
	return OnClientDisconnectCallback(pClient, p2);
}

//用户登录成功
bool CNetServerControl::SuccessLogin(basiclib::CBasicSessionNetClient* pNotify)
{
	pNotify->bind_rece(m_funcReceive);
	//加入用户队列
	pNotify->bind_disconnect(MakeFastFunction(this, &CNetServerControl::OnVerifyDisconnectCallback));
	uint32_t nSessionID = pNotify->GetSessionID();
	basiclib::CSingleLock lockUser(&m_mtxCSessionVerify, TRUE);
	basiclib::MapClientSession::iterator iter = m_mapClientSessionVerify.find(nSessionID);
	if (iter != m_mapClientSessionVerify.end()){
		basiclib::BasicLogEventErrorV("SuccessLogin 添加session失败 SessionID(%d)", pNotify->GetSessionID());
		pNotify->Close(0);
		return false;
	}
	m_mapClientSessionVerify[nSessionID] = pNotify;

	if (m_handleVerifySuccess)
		if (!m_handleVerifySuccess(pNotify)){
			pNotify->Close(0);
			return false;
		}
	return true;
}

basiclib::CBasicSessionNetClient* CNetServerControl::ConstructSession(uint32_t nSessionID)
{ 
	return CNetServerControlClient::CreateControlClient(nSessionID, this);
}

basiclib::CBasicSessionNetClient* CNetServerControl::CreateServerClientSession(uint32_t nSessionID)
{
    basiclib::CBasicSessionNetClient* pRet = CBasicSessionNetServer::CreateServerClientSession(nSessionID);
	pRet->bind_rece(MakeFastFunction(this, &CNetServerControl::OnUserVerify));
	return pRet;
}
