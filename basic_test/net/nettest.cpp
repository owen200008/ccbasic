#include "nettest.h"
#include "../scbasic/commu/servertemplate.h"
#include "../scbasic/commu/basicclient.h"
class CServerClient : public CNetServerControlClient
{
public:
	static CServerClient* CreateClient(uint32_t nSessionID, CRefNetServerControl pServer){ return new CServerClient(nSessionID, pServer); }
public:
	CServerClient(uint32_t nSessionID, CRefNetServerControl pServer) : CNetServerControlClient(nSessionID, pServer)
	{
		m_nRecv = 0;
	}
	virtual ~CServerClient()
	{
	}
protected:
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
        
		return BASIC_NET_HC_RET_HANDSHAKE;
	}
    virtual int32_t OnUserVerify(basiclib::CBasicSessionNetClient* pNotify, uint32_t dwNetCode, int32_t cbData, const char *pszData){
        if (memcmp(strInfo.c_str(), pszData, cbData) == 0 && cbData == strInfo.GetLength())
        {
            pNotify->Send((void*)pszData, cbData);
        }
        else
        {
			basiclib::BasicLogEvent(basiclib::DebugLevel_Info, "server verify fail");
            pNotify->Close();
            return BASIC_NET_OK;
        }
        return CNetServerControl::OnUserVerify(pNotify, dwNetCode, cbData, pszData);
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
				basiclib::BasicLogEventV(basiclib::DebugLevel_Info, "server receice packetnum error%d:%d", pIntData[i], pSession->m_nRecv);
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
		return BASIC_NET_OK;
	}
	int32_t bind_idlefunc(basiclib::CBasicSessionNetClient* pNotify, uint32_t dwNetState){
		return BASIC_NET_OK;
	}
	CServer(uint32_t nSessionID) : CNetServerControl(nSessionID)
	{
		bind_connect(MakeFastFunction(this, &CServer::bind_connectfunc));
        bind_rece(MakeFastFunction(this, &CServer::bind_recefuncsuccess));
		bind_disconnect(MakeFastFunction(this, &CServer::bind_disconnectfunc));
		bind_idle(MakeFastFunction(this, &CServer::bind_idlefunc));
	}
	virtual ~CServer(){}
	virtual basiclib::CBasicSessionNetClient* ConstructSession(uint32_t nSessionID){ return CServerClient::CreateClient(nSessionID, this); }
};

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
		basiclib::BasicLogEventV(basiclib::DebugLevel_Info, "%x Client:DisConnect(%s:%d)", this, strAddr.c_str(), nPort);
		return BASIC_NET_OK;
	}
	CClient(uint32_t nSessionID) : CCommonClientSession(nSessionID){
		m_nSendNumber = 0;
		m_nAlreadySendNumber = 0;
		m_nReceiveNumber = 0;
		bind_connect(MakeFastFunction(this, &CClient::bind_connectfunc));
		bind_rece(MakeFastFunction(this, &CClient::OnReceiveVerify));
		bind_idle(MakeFastFunction(this, &CClient::bind_idlefunc));
		bind_disconnect(MakeFastFunction(this, &CClient::bind_disconnectfunc));
	}
	virtual ~CClient(){}

	void DoSend(int nCount = 0){
		if (m_nSendNumber <= m_nReceiveNumber)
		{
			basiclib::BasicLogEventV(basiclib::DebugLevel_Info, "%x Client Send Finish:%d:%d", this, m_nAlreadySendNumber, m_nReceiveNumber);
			Close();
		}
		else
		{
			int nSendNumber = nCount == 0 ? ((rand() % 1000) + 100) : nCount;
			for (uint32_t i = 0; i < nSendNumber; i++)
			{
				Send((void*)&m_nAlreadySendNumber, sizeof(uint32_t));
				m_nAlreadySendNumber++;
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
				basiclib::BasicLogEventV(basiclib::DebugLevel_Info, "%x ClientPacketNumError %d:%d", this, pIntData[i], m_nReceiveNumber);
				pNotify->Close();
				return BASIC_NET_OK;
			}
			m_nReceiveNumber++;
		}
		basiclib::BasicLogEventV(basiclib::DebugLevel_Info, "%x ClientReceive %d/%d", this, m_nReceiveNumber, m_nSendNumber);
		DoSend(nCount);
		m_smRecvBuf.SetDataLength(0);
		return BASIC_NET_OK;
	}
	int32_t OnReceiveVerify(basiclib::CBasicSessionNetClient* pNotify, uint32_t dwNetState, int32_t cbData, const char* pData){
		if (memcmp(strInfo.c_str(), pData, cbData) == 0 && cbData == strInfo.GetLength())
		{
			m_nSendNumber = ((rand() % 65000) + 1) * 10;
			basiclib::BasicLogEventV(basiclib::DebugLevel_Info, "%x ClientSendNumber:%d\n", this, m_nSendNumber);
		}
        else{
            basiclib::BasicLogEvent(basiclib::DebugLevel_Info, "Client Verify fail!");
            Close(0);
            return BASIC_NET_OK;
        }

		bind_rece(MakeFastFunction(this, &CClient::OnReceive));
		DoSend(0);
		return BASIC_NET_HR_RET_HANDSHAKE;
	}
	
	uint32_t m_nSendNumber;
	uint32_t m_nAlreadySendNumber;
	uint32_t m_nReceiveNumber;
	basiclib::CBasicSmartBuffer m_smRecvBuf;
};


THREAD_RETURN WorkerClientThread(void* arg){
	srand(time(NULL) + basiclib::BasicGetTickTime());
	CClient* pSession = new CClient(rand());
	if (pSession->Connect(ADDRESS_C) != BASIC_NET_OK)
	{
		basiclib::BasicLogEventV(basiclib::DebugLevel_Info, "%x connect error:%s\n", pSession, ADDRESS_C);
	}
	int nIndex = 0;
	while (!bClose)
	{
		basiclib::BasicSleep(1000);
		if (pSession->GetSessionStatus(0xFFFFFFFF) == 0){
            if (rand() % 100 > 70 && nIndex < 5)
			{
				pSession->bind_rece(MakeFastFunction(pSession, &CClient::OnReceiveVerify));
				if (pSession->Connect(ADDRESS_C) != BASIC_NET_OK)
				{
					basiclib::BasicLogEventV(basiclib::DebugLevel_Info, "%x connect error:%s\n", pSession, ADDRESS_C);
				}
			}
			else
			{
				break;
			}
		}
		nIndex++;
	}
	basiclib::BasicLogEventV(basiclib::DebugLevel_Info, "WorkerClientThread Close");
	pSession->Release();
	return 0;
}

THREAD_RETURN WorkerServerThread(void *arg)
{
	srand(time(NULL) + basiclib::BasicGetTickTime() + basiclib::BasicGetCurrentThreadId());
	basiclib::SetNetInitializeGetParamFunc([](const char* pParam)->basiclib::CBasicString{
		//if (strcmp("NetThreadCount", pParam) == 0)
		//	return "4";
		return "";
	});
    int nCreateCount = 10;
	CServer* pServer = CServer::CreateServer();
	pServer->SetClientRecTimeout(30);
	int32_t lRet = pServer->Listen(ADDRESS_S, true);
	int nIndex = 0;
	while (!bClose)
	{
		basiclib::BasicSleep(1000);
        if (nIndex < nCreateCount)
		{
			DWORD dwThreadServerID = 0;
			basiclib::BasicCreateThread(WorkerClientThread, nullptr, &dwThreadServerID);
		}
		nIndex++;
	}
	pServer->Release();
	return 0;
}

void NetServerTest()
{
	srand(time(NULL) + basiclib::BasicGetTickTime());
	basiclib::BasicLogEvent(basiclib::DebugLevel_Info, "StartServer\n");
	DWORD dwThreadServerID = 0;
	basiclib::BasicCreateThread(WorkerServerThread, nullptr, &dwThreadServerID);

	getchar();
}

