#ifndef INC_BASKCTESTNET_H
#define INC_BASKCTESTNET_H

#include <basic.h>
#include "../scbasic/commu/servertemplate.h"
#include "../scbasic/commu/basicclient.h"
#include "../scbasic/encode/rsaencode.h"
LONG g_StartNetCount = 0;
typedef basiclib::CCheckNoPairKey<LONG> CheckNoReleaseInfo;
CheckNoReleaseInfo m_checknorelease;
//#define AddStack(key) m_checknorelease.Add(m_uniValue)
//#define DelStack(key) m_checknorelease.Del(m_uniValue)
#define AddStack(key) 
#define DelStack(key) 
class CServerClient : public CNetServerControlClient
{
public:
	static CServerClient* CreateClient(uint32_t nSessionID, CRefNetServerControl pServer){ return new CServerClient(nSessionID, pServer); }
public:
	CServerClient(uint32_t nSessionID, CRefNetServerControl pServer) : CNetServerControlClient(nSessionID, pServer)
	{
		m_uniValue = basiclib::BasicInterlockedIncrement(&g_StartNetCount);
		AddStack(m_uniValue);
		m_nRecv = 0;
	}
	virtual ~CServerClient()
	{
		basiclib::BasicInterlockedDecrement(&g_StartNetCount);
		DelStack(m_uniValue);
	}
protected:
	LONG	m_uniValue;
	int		m_nRecv;
	basiclib::CBasicSmartBuffer m_smRecvBuf;

	friend class CServer;
};

basiclib::CBasicString strInfo = "Vi";
class CServer : public CNetServerControl
{
public:
	static CServer* CreateServer(uint32_t nSessionID = 0){ return new CServer(nSessionID); }
protected:
	int32_t bind_connectfunc(basiclib::CBasicSessionNetClient* pNotify, uint32_t dwNetState){
		CServerClient* pSession = (CServerClient*)pNotify;
		basiclib::CBasicString strAddr;
		pSession->GetNetAddress(strAddr);
		uint32_t nPort = pSession->GetNetAddressPort();
		TRACE("Server:OnConnect(%s:%d)\n", strAddr.c_str(), nPort);
		return BASIC_NET_HC_RET_HANDSHAKE;
	}
	int32_t bind_recefunc(basiclib::CBasicSessionNetClient* pNotify, uint32_t dwNetState, int32_t cbData, const char* pData){
		if (memcmp(strInfo.c_str(), pData, cbData) == 0 && cbData == strInfo.GetLength())
		{
			pNotify->Send((void*)pData, cbData);
		}
		else
		{
			TRACE("认证包错误\n");
			pNotify->Close();
			return BASIC_NET_OK;
		}
		pNotify->bind_rece(MakeFastFunction(this, &CServer::bind_recefuncsuccess));
		return BASIC_NET_HR_RET_HANDSHAKE;
	}
	int32_t bind_recefuncsuccess(basiclib::CBasicSessionNetClient* pNotify, uint32_t dwNetState, int32_t cbData, const char* pData){
		CServerClient* pSession = (CServerClient*)pNotify;
		pSession->m_smRecvBuf.AppendData(pData, cbData);
		if (pSession->m_smRecvBuf.GetDataLength() % sizeof(int) != 0)
			return BASIC_NET_OK;
		int nCount = pSession->m_smRecvBuf.GetDataLength() / sizeof(int);
		int* pIntData = (int*)pSession->m_smRecvBuf.GetDataBuffer();
		for (int i = 0; i < nCount; i++)
		{
			if (pIntData[i] != pSession->m_nRecv)
			{
				TRACE("数字包错误%d:%d\n", pIntData[i], pSession->m_nRecv);
				pNotify->Close();
				return BASIC_NET_OK;
			}
			pSession->m_nRecv++;
		}
		pSession->Send((void*)pSession->m_smRecvBuf.GetDataBuffer(), pSession->m_smRecvBuf.GetDataLength());
		pSession->m_smRecvBuf.SetDataLength(0);
		return BASIC_NET_OK;
	}
	int32_t bind_disconnectfunc(basiclib::CBasicSessionNetClient* pNotify, uint32_t dwNetState){
		CServerClient* pSession = (CServerClient*)pNotify;
		basiclib::CBasicString strAddr;
		pSession->GetNetAddress(strAddr);
		uint32_t nPort = pSession->GetNetAddressPort();
		TRACE("Server:DisConnect(%s:%d)\n", strAddr.c_str(), nPort);
		return BASIC_NET_OK;
	}
	int32_t bind_idlefunc(basiclib::CBasicSessionNetClient* pNotify, uint32_t dwNetState){
		if (dwNetState % 10 == 0)
		{
			TRACE("Server:Idle(%d)\n", dwNetState);
		}
		return BASIC_NET_OK;
	}
	int32_t bind_errorfunc(basiclib::CBasicSessionNetClient* pNotify, uint32_t dwNetState, int32_t lError){
		CServerClient* pSession = (CServerClient*)pNotify;
		TRACE("Server:Error(%d:%d)\n", dwNetState, lError);
		return BASIC_NET_OK;
	}
	CServer(uint32_t nSessionID) : CNetServerControl(nSessionID)
	{
		m_uniValue = basiclib::BasicInterlockedIncrement(&g_StartNetCount);
		AddStack(m_uniValue);
		bind_connect(MakeFastFunction(this, &CServer::bind_connectfunc));
		bind_rece(MakeFastFunction(this, &CServer::bind_recefunc));
		bind_disconnect(MakeFastFunction(this, &CServer::bind_disconnectfunc));
		bind_idle(MakeFastFunction(this, &CServer::bind_idlefunc));
		bind_error(MakeFastFunction(this, &CServer::bind_errorfunc));
	}
	virtual ~CServer()
	{
		basiclib::BasicInterlockedDecrement(&g_StartNetCount);
		DelStack(m_uniValue);
	}

	virtual basiclib::CBasicSessionNetClient* ConstructSession(uint32_t nSessionID){ return CServerClient::CreateClient(nSessionID, this); }

	LONG m_uniValue;
};

int nDumpTick = 0;
bool bDump = false;

#define ADDRESS_S "0.0.0.0:8000"
#define ADDRESS_C "127.0.0.1:8000"
bool bClose = false;

class CClient : public CCommonClientSession
{
public:
	int32_t bind_connectfunc(basiclib::CBasicSessionNetClient* pNotify, uint32_t dwNetState){
		m_nSendNumber = 0;
		m_nAlreadySendNumber = 0;
		m_nReceiveNumber = 0;
		pNotify->Send((void*)strInfo.c_str(), strInfo.GetLength());
		return BASIC_NET_OK;
	}
	int32_t bind_idlefunc(basiclib::CBasicSessionNetClient* pNotify, uint32_t dwNetState){
		return BASIC_NET_OK;
	}
	int32_t bind_disconnectfunc(basiclib::CBasicSessionNetClient* pNotify, uint32_t dwNetState){
		basiclib::CBasicString strAddr;
		pNotify->GetNetAddress(strAddr);
		uint32_t nPort = pNotify->GetNetAddressPort();
		TRACE("%x Client:DisConnect(%s:%d)\n", this, strAddr.c_str(), nPort);
		return BASIC_NET_OK;
	}
	CClient(uint32_t nSessionID) : CCommonClientSession(nSessionID){
		m_uniValue = basiclib::BasicInterlockedIncrement(&g_StartNetCount);
		AddStack(m_uniValue);
		m_nSendNumber = 0;
		m_nAlreadySendNumber = 0;
		m_nReceiveNumber = 0;
		bind_connect(MakeFastFunction(this, &CClient::bind_connectfunc));
		bind_rece(MakeFastFunction(this, &CClient::OnReceiveVerify));
		bind_idle(MakeFastFunction(this, &CClient::bind_idlefunc));
		bind_disconnect(MakeFastFunction(this, &CClient::bind_disconnectfunc));
	}
	virtual ~CClient(){
		basiclib::BasicInterlockedDecrement(&g_StartNetCount);
		DelStack(m_uniValue);
	}

	void DoSend(int nCount = 0){
		if (m_nSendNumber <= m_nReceiveNumber)
		{
			TRACE("%x Client Send Finish:%d:%d\n", this, m_nAlreadySendNumber, m_nReceiveNumber);
			Close();
			bDump = true;
		}
		else
		{
			//int nSendNumber = ((rand() + 1)) % (m_nSendNumber - m_nAlreadySendNumber) + 1;
			int nSendNumber = nCount == 0 ? ((rand() + 1)) % 1000 : nCount;
			for (uint32_t i = 0; i < nSendNumber; i++)
			{
				Send((void*)&m_nAlreadySendNumber, sizeof(uint32_t));
				m_nAlreadySendNumber++;
				//if (m_nAlreadySendNumber % 10000 == 9999)
				//	TRACE("%x Client Send:%d:%d\n", this, m_nAlreadySendNumber, nSendNumber);
			}
		}
	}
	int32_t OnReceive(basiclib::CBasicSessionNetClient* pNotify, uint32_t dwNetState, int32_t cbData, const char* pData){
		m_smRecvBuf.AppendData(pData, cbData);
		if (m_smRecvBuf.GetDataLength() % sizeof(int) != 0)
			return BASIC_NET_OK;
		int nCount = m_smRecvBuf.GetDataLength() / sizeof(int);
		int* pIntData = (int*)m_smRecvBuf.GetDataBuffer();
		for (int i = 0; i < nCount; i++)
		{
			if (pIntData[i] != m_nReceiveNumber)
			{
				TRACE("%x Client数字包错误%d:%d\n", this, pIntData[i], m_nReceiveNumber);
				pNotify->Close();
				return BASIC_NET_OK;
			}
			m_nReceiveNumber++;
		}
		DoSend(nCount);
		m_smRecvBuf.SetDataLength(0);
		return BASIC_NET_OK;
	}
	int32_t OnReceiveVerify(basiclib::CBasicSessionNetClient* pNotify, uint32_t dwNetState, int32_t cbData, const char* pData){
		if (memcmp(strInfo.c_str(), pData, cbData) == 0 && cbData == strInfo.GetLength())
		{
			m_nSendNumber = (rand() + 1) * 10;
			TRACE("%x ClientSendNumber:%d\n", this, m_nSendNumber);
		}

		bind_rece(MakeFastFunction(this, &CClient::OnReceive));
		DoSend(0);
		return BASIC_NET_HR_RET_HANDSHAKE;
	}
	

	uint32_t m_nSendNumber;
	uint32_t m_nAlreadySendNumber;
	uint32_t m_nReceiveNumber;
	basiclib::CBasicSmartBuffer m_smRecvBuf;
	LONG m_uniValue;
};


THREAD_RETURN WorkerClientThread(void* arg){
	LONG m_uniValue = basiclib::BasicInterlockedIncrement(&g_StartNetCount);
	AddStack(m_uniValue);
	srand(time(NULL) + basiclib::BasicGetTickTime());
	CClient* pSession = new CClient(m_uniValue);
	if (pSession->Connect(ADDRESS_C) != BASIC_NET_OK)
	{
		TRACE("%x connect error:%s\n", pSession, ADDRESS_C);
	}
	int nIndex = 0;
	while (!bClose)
	{
		basiclib::BasicSleep(1000);
		nIndex++;
		if (pSession->GetSessionStatus(0xFFFFFFFF) == 0){
			if (rand() % 100 > 70)
			{
				pSession->bind_rece(MakeFastFunction(pSession, &CClient::OnReceiveVerify));
				if (pSession->Connect(ADDRESS_C) != BASIC_NET_OK)
				{
					TRACE("%x connect error:%s\n", pSession, ADDRESS_C);
				}
			}
			else
			{
				break;
			}
		}
	}
	pSession->Release();
	basiclib::BasicInterlockedDecrement(&g_StartNetCount);
	DelStack(m_uniValue);
	return 0;
}

THREAD_RETURN WorkerServerThread(void *arg)
{
	LONG m_uniValue = basiclib::BasicInterlockedIncrement(&g_StartNetCount);
	AddStack(m_uniValue);
	srand(time(NULL) + basiclib::BasicGetTickTime() + basiclib::BasicGetCurrentThreadId());
	basiclib::SetNetInitializeGetParamFunc([](const char* pParam)->basiclib::CBasicString{
		if (strcmp("ThreadCount", pParam) == 0)
			return "4";
		return "";
	});
	CServer* pServer = CServer::CreateServer();
	pServer->SetClientRecTimeout(30);
	int32_t lRet = pServer->Listen(ADDRESS_S, true);
	//TRACE("ListenRet:%d %s\n", lRet, pServer->GetLibeventMethod());
	int nIndex = 0;
	while (!bClose)
	{
		basiclib::BasicSleep(1000);
		nIndex++;
		if (nIndex % 10 == 9){
			size_t a, b, c, e, f;
			basiclib::BasicGetOperationInfo(a, b, c, e, f);
			TRACE("Count(%d) Mem:(allocatecount:%d, deallocate:%d, Use:%d, %d, %d)\n", pServer->GetOnlineSessionCount(), a, b, c, e, f);
		}
		//if (nIndex == 1)
		{
			DWORD dwThreadServerID = 0;
			basiclib::BasicCreateThread(WorkerClientThread, nullptr, &dwThreadServerID);
		}
	}
	pServer->Release();
	basiclib::BasicInterlockedDecrement(&g_StartNetCount);
	DelStack(m_uniValue);
	return 0;
}

void NetServerTest()
{
	CSCBasicRSA rsa;
	rsa.SetPublicFileName("e:/pub.gem");

	basiclib::BasicSetMemRunMemCheck(MemRunMemCheck_RunTongJi | MemRunMemCheck_RunCheckMem, 9, 0x0FFFFFFF);
	srand(time(NULL) + basiclib::BasicGetTickTime());
	TRACE("StartServer\n");
	DWORD dwThreadServerID = 0;
	basiclib::BasicCreateThread(WorkerServerThread, nullptr, &dwThreadServerID);

	getchar();
	TRACE("SetClose\n");
	bClose = true;
	basiclib::DumpRunMemCheck();
	while (g_StartNetCount != 0){
		bool bFalse = false;
		if (bFalse)
		{
			m_checknorelease.Dump([](LONG, stacktrace::call_stack& stack)->void{
				basiclib::BasicLogEvent(stack.to_string().c_str());
			});
		}
		basiclib::BasicSleep(1000);
	}
	size_t a, b, c, e, f;
	basiclib::BasicGetOperationInfo(a, b, c, e, f);
	TRACE("Mem:(allocatecount:%d, deallocate:%d, Use:%d, %d, %d)\n", a, b, c, e, f);
	basiclib::DumpRunMemCheck();
	TRACE("Closed\n");
}

#endif