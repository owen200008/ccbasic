#include "servertemplate.h"

CNetServerControlClient::CNetServerControlClient(Net_UInt nSessionID, CRefNetServerControl pServer) : basiclib::CBasicSessionNetClient(nSessionID, false)
{
	m_server = pServer;
}
CNetServerControlClient::~CNetServerControlClient()
{

}

Net_Int CNetServerControlClient::OnConnect(Net_UInt dwNetCode)
{
	if (!m_server->m_ipTrust.IsIpTrust(m_szPeerAddr))
	{
		basiclib::BasicLogEventV("地址：%s 不在白名单内!", m_szPeerAddr);
		Close();
		return BASIC_NET_OK;
	}

	if (m_server->m_nSessionMaxCount >= 0)
	{
		if (m_server->GetOnlineSessionCount() > m_server->m_nSessionMaxCount)
		{
			basiclib::BasicLogEventV("地址：%s 超过最大连接数 %d!", m_szPeerAddr, m_server->m_nSessionMaxCount);
			Close();
			return BASIC_NET_OK;
		}
	}
	//需要认证
	return basiclib::CBasicSessionNetClient::OnConnect(dwNetCode);
}
Net_Int CNetServerControlClient::OnDisconnect(Net_UInt dwNetCode)
{
	return basiclib::CBasicSessionNetClient::OnDisconnect(dwNetCode);
}
Net_Int CNetServerControlClient::OnReceive(Net_UInt dwNetCode, const char *pszData, Net_Int cbData)
{
	return basiclib::CBasicSessionNetClient::OnReceive(dwNetCode, pszData, cbData);
}
//////////////////////////////////////////////////////////////////////////////////////
CNetServerControl::CNetServerControl(Net_UInt nSessionID) : basiclib::CBasicSessionNetServer(nSessionID)
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

void CNetServerControl::OnTimer(Net_UInt nTick)
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
			basiclib::BasicLogEventV("ListenPort: [%s] Success!", m_strListenAddr.c_str());
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

Net_Int CNetServerControl::StartServer(const char* lpszAddress, basiclib::CBasicPreSend* pPreSend)
{
	if (pPreSend)
	{
		RegistePreSend(pPreSend);
	}
	Net_Int nRet = Listen(lpszAddress, true);
	if (nRet == BASIC_NET_OK)
	{
		basiclib::BasicLogEventV("ListenPort: %s Success!", lpszAddress);
	}
	else
	{
		Close();
		basiclib::BasicLogEventV("ListenPort: %s Error!", lpszAddress);
	}
	return nRet;
}

Net_Int CNetServerControl::OnUserVerify(basiclib::CBasicSessionNetClient* pNotify, Net_UInt dwNetCode, Net_Int cbData, const char *pszData)
{
	//先不需要回调上层
	SuccessLogin(pNotify);
	return BASIC_NET_OK;
}

Net_Int CNetServerControl::OnVerifyDisconnectCallback(basiclib::CBasicSessionNetClient* pClient, Net_UInt p2)
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
	Net_UInt nSessionID = pNotify->GetSessionID();
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

basiclib::CBasicSessionNetClient* CNetServerControl::ConstructSession(Net_UInt nSessionID)
{ 
	return CNetServerControlClient::CreateControlClient(nSessionID, this);
}

basiclib::CBasicSessionNetClient* CNetServerControl::CreateServerClientSession(Net_UInt nSessionID)
{
	basiclib::CBasicSessionNetClient* pRet = ConstructSession(nSessionID);
	pRet->bind_rece(MakeFastFunction(this, &CNetServerControl::OnUserVerify));
	return pRet;
}
