#include "mysqlconnector_netwapper.h"
/////////////////////////////////////////////////////////////////////////////////////////////
CMySQLConnector_NetWapperBasicClient::CMySQLConnector_NetWapperBasicClient()
{
}

CMySQLConnector_NetWapperBasicClient::~CMySQLConnector_NetWapperBasicClient()
{

}

bool CMySQLConnector_NetWapperBasicClient::InitNetWapper()
{
	m_pNet = CCommonClientSession::CreateCCommonClientSession();
	m_pNet->bind_rece(MakeFastFunction(this, &CMySQLConnector_NetWapperBasicClient::OnNetReceiveData));
	m_pNet->bind_disconnect(MakeFastFunction(this, &CMySQLConnector_NetWapperBasicClient::OnNetDisconnect));
    m_pNet->bind_idle(MakeFastFunction(this, &CMySQLConnector_NetWapperBasicClient::OnNetIdle));
	return true;
}

Net_Int CMySQLConnector_NetWapperBasicClient::OnNetReceiveData(basiclib::CBasicSessionNetClient* pNotify, Net_UInt dwNetCode, Net_Int cbData, const char* pData)
{
	if (!m_notifyFunc(NotifyNetWapper_Recv, (void*)pData, (void*)&cbData)){
		basiclib::BasicLogEventErrorV("CMySQLConnector_NetWapperBasicClient::OnNetReceiveData Error Recv");
		return BASIC_NET_GENERIC_ERROR;
	}
	return BASIC_NET_OK;
}
Net_Int CMySQLConnector_NetWapperBasicClient::OnNetDisconnect(basiclib::CBasicSessionNetClient*, Net_UInt dwNetCode)
{
	if (0 == dwNetCode){
		//如果是主动断开就不提示
		m_notifyFunc(NotifyNetWapper_SelfDisconnect, nullptr, nullptr);
	}
	else if (0x40 & dwNetCode){
		m_notifyFunc(NotifyNetWapper_ConnectFail, nullptr, nullptr);
	}
	else if (0x2 & dwNetCode){
		// Close by server.
		m_notifyFunc(NotifyNetWapper_RemoteDisconnect, nullptr, nullptr);
	}
	else{
		// Close by server.
		m_notifyFunc(NotifyNetWapper_RemoteDisconnect, nullptr, nullptr);
	}
	return BASIC_NET_OK;
}

Net_Int CMySQLConnector_NetWapperBasicClient::OnNetIdle(basiclib::CBasicSessionNetClient*, Net_UInt nIdle){
    if (nIdle % 15 == 14){
        //15s通知一次
        m_notifyFunc(NotifyNetWapper_Idle, nullptr, nullptr);
    }
}

bool CMySQLConnector_NetWapperBasicClient::ConnectToServer(const char* pAddressPort)
{
	Net_Int nRet = m_pNet->Connect(pAddressPort);
	return nRet == BASIC_NET_OK;
}

bool CMySQLConnector_NetWapperBasicClient::ReconnectToServer()
{
	return m_pNet->DoConnect() == BASIC_NET_OK;
}

void CMySQLConnector_NetWapperBasicClient::SendDataToServer(const char* pData, int nLength)
{
	m_pNet->Send((void*)pData, nLength);
}

void CMySQLConnector_NetWapperBasicClient::DisconnectToServer()
{
	m_pNet->Close();
}
bool CMySQLConnector_NetWapperBasicClient::IsTransmit()
{
	return m_pNet->IsTransmit();
}