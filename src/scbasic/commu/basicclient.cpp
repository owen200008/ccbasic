#include "basicclient.h"

using namespace basiclib;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CCommonClientSession::CCommonClientSession(Net_UInt nSessionID) : basiclib::CBasicSessionNetClient(nSessionID, true)
{
}

CCommonClientSession::~CCommonClientSession()
{

}

//！ 自身不存在定时器,由外部调用
void CCommonClientSession::OnTimer(Net_UInt nTickTime)
{
	basiclib::CBasicSessionNetClient::OnTimer(nTickTime);
}

//! 开始连接
Net_Int CCommonClientSession::Connect(const char* lpszAddress)
{
	BasicLogEventV("start connect to server IP:%s........", lpszAddress);
	Net_Int nRet = basiclib::CBasicSessionNetClient::Connect(lpszAddress);
	if (nRet != BASIC_NET_OK)
	{
		BasicLogEventV("connect to server IP:%s failed, the error id: %d", lpszAddress, nRet);
	}
	return nRet;
}
Net_Int CCommonClientSession::OnDisconnect(Net_UInt dwNetCode)
{
	//日志记录
	BasicLogEventV("disconnect to server IP:%s", m_strConnectAddr.c_str());
	return basiclib::CBasicSessionNetClient::OnDisconnect(dwNetCode);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
