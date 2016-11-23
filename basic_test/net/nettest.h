#ifndef INC_BASKCTESTNET_H
#define INC_BASKCTESTNET_H

#include <basic.h>
#include "../scbasic/commu/servertemplate.h"
#include "../scbasic/commu/basicclient.h"
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
	static CServerClient* CreateClient(Net_UInt nSessionID, CRefNetServerControl pServer){ return new CServerClient(nSessionID, pServer); }
public:
	CServerClient(Net_UInt nSessionID, CRefNetServerControl pServer) : CNetServerControlClient(nSessionID, pServer)
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
	static CServer* CreateServer(Net_UInt nSessionID = 0){ return new CServer(nSessionID); }
protected:
	Net_Int bind_connectfunc(basiclib::CBasicSessionNetClient* pNotify, Net_UInt dwNetState){
		CServerClient* pSession = (CServerClient*)pNotify;
		basiclib::CBasicString strAddr;
		pSession->GetNetAddress(strAddr);
		UINT nPort = pSession->GetNetAddressPort();
		TRACE("Server:OnConnect(%s:%d)\n", strAddr.c_str(), nPort);
		return BASIC_NET_HC_RET_HANDSHAKE;
	}
	Net_Int bind_recefunc(basiclib::CBasicSessionNetClient* pNotify, Net_UInt dwNetState, Net_Int cbData, const char* pData){
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
	Net_Int bind_recefuncsuccess(basiclib::CBasicSessionNetClient* pNotify, Net_UInt dwNetState, Net_Int cbData, const char* pData){
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
	Net_Int bind_disconnectfunc(basiclib::CBasicSessionNetClient* pNotify, Net_UInt dwNetState){
		CServerClient* pSession = (CServerClient*)pNotify;
		basiclib::CBasicString strAddr;
		pSession->GetNetAddress(strAddr);
		UINT nPort = pSession->GetNetAddressPort();
		TRACE("Server:DisConnect(%s:%d)\n", strAddr.c_str(), nPort);
		return BASIC_NET_OK;
	}
	Net_Int bind_idlefunc(basiclib::CBasicSessionNetClient* pNotify, Net_UInt dwNetState){
		if (dwNetState % 10 == 0)
		{
			TRACE("Server:Idle(%d)\n", dwNetState);
		}
		return BASIC_NET_OK;
	}
	Net_Int bind_errorfunc(basiclib::CBasicSessionNetClient* pNotify, Net_UInt dwNetState, Net_Int lError){
		CServerClient* pSession = (CServerClient*)pNotify;
		TRACE("Server:Error(%d:%d)\n", dwNetState, lError);
		return BASIC_NET_OK;
	}
	CServer(Net_UInt nSessionID) : CNetServerControl(nSessionID)
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

	virtual basiclib::CBasicSessionNetClient* ConstructSession(Net_UInt nSessionID){ return CServerClient::CreateClient(nSessionID, this); }

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
	Net_Int bind_connectfunc(basiclib::CBasicSessionNetClient* pNotify, Net_UInt dwNetState){
		m_nSendNumber = 0;
		m_nAlreadySendNumber = 0;
		m_nReceiveNumber = 0;
		pNotify->Send((void*)strInfo.c_str(), strInfo.GetLength());
		return BASIC_NET_OK;
	}
	Net_Int bind_idlefunc(basiclib::CBasicSessionNetClient* pNotify, Net_UInt dwNetState){
		return BASIC_NET_OK;
	}
	Net_Int bind_disconnectfunc(basiclib::CBasicSessionNetClient* pNotify, Net_UInt dwNetState){
		basiclib::CBasicString strAddr;
		pNotify->GetNetAddress(strAddr);
		UINT nPort = pNotify->GetNetAddressPort();
		TRACE("%x Client:DisConnect(%s:%d)\n", this, strAddr.c_str(), nPort);
		return BASIC_NET_OK;
	}
	CClient(Net_UInt nSessionID) : CCommonClientSession(nSessionID){
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
			for (Net_UInt i = 0; i < nSendNumber; i++)
			{
				Send((void*)&m_nAlreadySendNumber, sizeof(Net_UInt));
				m_nAlreadySendNumber++;
				//if (m_nAlreadySendNumber % 10000 == 9999)
				//	TRACE("%x Client Send:%d:%d\n", this, m_nAlreadySendNumber, nSendNumber);
			}
		}
	}
	Net_Int OnReceive(basiclib::CBasicSessionNetClient* pNotify, Net_UInt dwNetState, Net_Int cbData, const char* pData){
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
	Net_Int OnReceiveVerify(basiclib::CBasicSessionNetClient* pNotify, Net_UInt dwNetState, Net_Int cbData, const char* pData){
		if (memcmp(strInfo.c_str(), pData, cbData) == 0 && cbData == strInfo.GetLength())
		{
			m_nSendNumber = (rand() + 1) * 10;
			TRACE("%x ClientSendNumber:%d\n", this, m_nSendNumber);
		}

		bind_rece(MakeFastFunction(this, &CClient::OnReceive));
		DoSend(0);
		return BASIC_NET_HR_RET_HANDSHAKE;
	}
	

	Net_UInt m_nSendNumber;
	Net_UInt m_nAlreadySendNumber;
	Net_UInt m_nReceiveNumber;
	basiclib::CBasicSmartBuffer m_smRecvBuf;
	LONG m_uniValue;
};


THREAD_RETURN WorkerClientThread(void* arg){
	LONG m_uniValue = basiclib::BasicInterlockedIncrement(&g_StartNetCount);
	AddStack(m_uniValue);
	srand(time(NULL) + basiclib::BasicGetTickTime());
	CClient* pSession = new CClient(rand());
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
	Net_Int lRet = pServer->Listen(ADDRESS_S, true);
	TRACE("ListenRet:%d %s\n", lRet, pServer->GetLibeventMethod());
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
	basiclib::BasicSetMemRunMemCheck(0x00000002, 9, 0x0FFFFFFF);
	srand(time(NULL) + basiclib::BasicGetTickTime());
	TRACE("StartServer\n");
	DWORD dwThreadServerID = 0;
	basiclib::BasicCreateThread(WorkerServerThread, nullptr, &dwThreadServerID);

	getchar();
	TRACE("SetClose\n");
	bClose = true;
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
	getchar();
}

#endif