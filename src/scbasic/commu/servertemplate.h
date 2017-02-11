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
	typedef fastdelegate::FastDelegate1<basiclib::CBasicSessionNetClient*, bool> HandleVerifySuccess;

	static CNetServerControl* CreateNetServerControl(uint32_t nSessionID = 0){ return new CNetServerControl(nSessionID); }
protected:
	CNetServerControl(uint32_t nSessionID);
	virtual ~CNetServerControl();
public:
	void bind_verifysuccess(const HandleVerifySuccess& func){ m_handleVerifySuccess = func; }

	virtual int32_t StartServer(const char* lpszAddress, basiclib::CBasicPreSend* pPreSend = nullptr);
	BOOL IsListen();
	void SetIpTrust(const char* lpszIpTrust);
	void SetSessionMaxCount(int nCount);

	//不需要外部调用
	virtual void OnTimer(uint32_t nTick);
protected:
	virtual int32_t OnUserVerify(basiclib::CBasicSessionNetClient* pNotify, uint32_t dwNetCode, int32_t cbData, const char *pszData);
	virtual int32_t OnVerifyDisconnectCallback(basiclib::CBasicSessionNetClient* pClient, uint32_t p2);
	//用户登录成功
	virtual bool SuccessLogin(basiclib::CBasicSessionNetClient* pNotify);
protected:
	virtual basiclib::CBasicSessionNetClient* CreateServerClientSession(uint32_t nSessionID);
	virtual basiclib::CBasicSessionNetClient* ConstructSession(uint32_t nSessionID);
protected:
	HandleVerifySuccess			m_handleVerifySuccess;
	//ip信任地址
	CIpVerify					m_ipTrust;
	// 最大允许session连接数量	小于0 表示不限制
	int							m_nSessionMaxCount;

	basiclib::CMutex			m_mtxCSessionVerify;
	basiclib::MapClientSession	m_mapClientSessionVerify;

	friend class CNetServerControlClient;
};

typedef basiclib::CBasicRefPtr<CNetServerControl> CRefNetServerControl;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CNetServerControlClient : public basiclib::CBasicSessionNetClient
{
public:
	static CNetServerControlClient* CreateControlClient(uint32_t nSessionID, CRefNetServerControl pServer){ return new CNetServerControlClient(nSessionID, pServer); }
protected:
	CNetServerControlClient(uint32_t nSessionID, CRefNetServerControl pServer);
	virtual ~CNetServerControlClient();

	virtual int32_t OnConnect(uint32_t dwNetCode);
	virtual int32_t OnDisconnect(uint32_t dwNetCode);
	virtual int32_t OnReceive(uint32_t dwNetCode, const char *pszData, int32_t cbData);
public:
	CRefNetServerControl m_server;
};

#endif

