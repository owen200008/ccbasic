#include "basicclient.h"

using namespace basiclib;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CCommonClientSession::CCommonClientSession(uint32_t nSessionID) : basiclib::CBasicSessionNetClient(nSessionID, true)
{
}

CCommonClientSession::~CCommonClientSession()
{

}

//! 开始连接
int32_t CCommonClientSession::Connect(const char* lpszAddress)
{
	BasicLogEventV("start connect to server IP:%s........", lpszAddress);
	int32_t nRet = basiclib::CBasicSessionNetClient::Connect(lpszAddress);
	if (nRet != BASIC_NET_OK)
	{
		BasicLogEventV("connect to server IP:%s failed, the error id: %d", lpszAddress, nRet);
	}
	return nRet;
}
int32_t CCommonClientSession::OnDisconnect(uint32_t dwNetCode)
{
	//日志记录
	BasicLogEventV("disconnect to server IP:%s", m_strConnectAddr.c_str());
	return basiclib::CBasicSessionNetClient::OnDisconnect(dwNetCode);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
