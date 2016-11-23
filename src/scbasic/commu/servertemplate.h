/////////////////////////////////////////////////////////////////////////////////////////////
// 文件名:		servertemplate.h
// 创建者:		蔡振球
// 创建时间:	2010/07/07
// 内容描述:	服务程序的模板
/////////////////////////////////////////////////////////////////////////////////////////////
#ifndef INC_SERVERTEMPLATE_H
#define INC_SERVERTEMPLATE_H

#include "ipverify.h"

class CNetServerControl : public basiclib::CBasicSessionNetServer
{
public:
	static CNetServerControl* CreateNetServerControl(Net_UInt nSessionID){ return new CNetServerControl(nSessionID); }
protected:
	CNetServerControl(Net_UInt nSessionID);
	virtual ~CNetServerControl();
public:
	Net_Int StartServer(const char* lpszAddress, basiclib::CBasicPreSend* pPreSend = nullptr);
	BOOL IsListen();
	void SetIpTrust(const char* lpszIpTrust);
	void SetSessionMaxCount(int nCount);

	//不需要外部调用
	virtual void OnTimer(Net_UInt nTick);
protected:
	virtual basiclib::CBasicSessionNetClient* ConstructSession(Net_UInt nSessionID);
protected:
	//ip信任地址
	CIpVerify			m_ipTrust;
	// 最大允许session连接数量	小于0 表示不限制
	int	m_nSessionMaxCount;

	friend class CNetServerControlClient;
};

typedef basiclib::CBasicRefPtr<CNetServerControl> CRefNetServerControl;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CNetServerControlClient : public basiclib::CBasicSessionNetClient
{
public:
	static CNetServerControlClient* CreateControlClient(Net_UInt nSessionID, CRefNetServerControl pServer){ return new CNetServerControlClient(nSessionID, pServer); }
protected:
	CNetServerControlClient(Net_UInt nSessionID, CRefNetServerControl pServer);
	virtual ~CNetServerControlClient();

	virtual Net_Int OnConnect(Net_UInt dwNetCode);
	virtual Net_Int OnDisconnect(Net_UInt dwNetCode);
	virtual Net_Int OnReceive(Net_UInt dwNetCode, const char *pszData, Net_Int cbData);
public:
	CRefNetServerControl m_server;
};

#endif

