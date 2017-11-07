#include "basicclient.h"

using namespace basiclib;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! 开始连接
int32_t CCommonClientSession::Connect(const char* lpszAddress){
    BasicLogEventV(DebugLevel_Info, "start connect to server IP:%s........", lpszAddress);
	int32_t nRet = basiclib::CBasicSessionNetClient::Connect(lpszAddress);
	if (nRet != BASIC_NET_OK){
        BasicLogEventV(DebugLevel_Info, "connect to server IP:%s failed, the error id: %d", lpszAddress, nRet);
	}
	return nRet;
}
uint32_t CCommonClientSession::OnDisconnect(uint32_t dwNetCode){
	//日志记录
    BasicLogEventV(DebugLevel_Info, "disconnect to server IP:%s(%d)", GetConnectAddr().c_str(), dwNetCode);
	return basiclib::CBasicSessionNetClient::OnDisconnect(dwNetCode);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
