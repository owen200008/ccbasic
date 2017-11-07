#ifndef _MYSQL_CONNECTOR_NETWAPPER_H_
#define _MYSQL_CONNECTOR_NETWAPPER_H_

#include "../scbasic/commu/basicclient.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum NotifyNetWapper
{
	NotifyNetWapper_Connected = 0,
	NotifyNetWapper_Recv = 1,
	NotifyNetWapper_ConnectFail = 2,
	NotifyNetWapper_RemoteDisconnect = 3,
	NotifyNetWapper_SelfDisconnect = 4,
    NotifyNetWapper_Idle = 5,
};
typedef fastdelegate::FastDelegate3<NotifyNetWapper, void*, void*, bool> OnNotifyNetWapper;
class CMySQLConnector_NetWapper : public basiclib::CBasicObject
{
public:
	//! 初始化
	virtual bool InitNetWapper() = 0;
	//! 连接mysql服务器
	virtual bool ConnectToServer(const char* pAddressPort) = 0;
	//! 重连mysql服务器
	virtual bool ReconnectToServer() = 0;
	//! 断开mysql的连接
	virtual void DisconnectToServer() = 0;
	//! 发送数据
	virtual void SendDataToServer(const char* pData, int nLength) = 0;
	//! 绑定接收包
	void BindNotifyNetWapper(OnNotifyNetWapper func){
		m_notifyFunc = func;
	}
	//! 判断网络是否可以传输
	virtual bool IsTransmit() = 0;
protected:
	OnNotifyNetWapper			m_notifyFunc;
};

class CMySQLConnector_NetWapperBasicClient : public CMySQLConnector_NetWapper
{
public:
	CMySQLConnector_NetWapperBasicClient();
	virtual ~CMySQLConnector_NetWapperBasicClient();

	virtual bool InitNetWapper();
	virtual bool ConnectToServer(const char* pAddressPort);
	virtual bool ReconnectToServer();
	virtual void DisconnectToServer();
	virtual void SendDataToServer(const char* pData, int nLength);
	virtual bool IsTransmit();
protected:
	Net_Int OnNetReceiveData(basiclib::CBasicSessionNetClient*, Net_UInt, Net_Int, const char *);
	Net_Int OnNetDisconnect(basiclib::CBasicSessionNetClient*, Net_UInt);
    Net_Int OnNetIdle(basiclib::CBasicSessionNetClient*, Net_UInt);
protected:
	CCommonClientSession*	m_pNet;
};



#endif