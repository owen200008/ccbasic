#include "nettest.h"
#include "../scbasic/commu/servertemplate.h"
#include "../scbasic/commu/basicclient.h"

void TestCallFunc(){
	printf("ASCII\n");
}
void TestCallFuncW(){
	printf("UNICODE\n");
}
#define TestCallFuncA     TestCallFunc

#ifdef UNICODE
#define TestCallFunc      TestCallFuncW
#endif

class CServerClient : public CNetServerControlSession
{
	DefineCreateNetServerSessionWithServer(CServerClient)
protected:
	CServerClient(){
		m_nRecv = 0;
	}
	virtual ~CServerClient(){
	}
protected:
	int		m_nRecv;
	basiclib::CBasicSmartBuffer m_smRecvBuf;
	friend class CServer;
};

basiclib::CBasicString strInfo = "Vi";
class CServer : public CNetServerControl
{
	DefineCreateNetServerDefault(CServer)
protected:
	virtual basiclib::CBasicSessionNetServerSession* CreateServerClientSession(uint32_t nSessionID){
		basiclib::CBasicSessionNetServerSession* pNotify = CNetServerControl::CreateServerClientSession(nSessionID);
		pNotify->bind_rece(MakeFastFunction(this, &CServer::OnUserVerify));
		return pNotify;
	}
	virtual basiclib::CBasicSessionNetServerSession* ConstructSession(Net_UInt nSessionID){
		return CServerClient::CreateNetServerSessionWithServer(nSessionID, m_usRecTimeout, this);
	}

	//! 绑定认证成功，就不需要connect
	int32_t OnUserConnect(basiclib::CBasicSessionNetNotify* pNotify, Net_UInt dwNetCode){
		return BASIC_NET_HC_RET_HANDSHAKE;
	}
    int32_t OnUserVerify(basiclib::CBasicSessionNetNotify* pNotify, uint32_t dwNetCode, int32_t cbData, const char *pszData){
        if (memcmp(strInfo.c_str(), pszData, strInfo.GetLength()) == 0){
            pNotify->Send((void*)pszData, cbData);
        }
        else{
			basiclib::BasicLogEvent(basiclib::DebugLevel_Info, "server verify fail");
            pNotify->Close();
            return BASIC_NET_OK;
        }
		CServerClient* pSession = (CServerClient*)pNotify;
		pSession->SuccessLogin();
        pNotify->bind_rece(MakeFastFunction(this, &CServer::bind_recefuncsuccess));
		if(cbData - strInfo.GetLength() > 0){
			bind_recefuncsuccess(pNotify, dwNetCode, cbData - strInfo.GetLength(), pszData + strInfo.GetLength());
		}
        return BASIC_NET_HR_RET_HANDSHAKE;
    }
	int32_t bind_recefuncsuccess(basiclib::CBasicSessionNetNotify* pNotify, uint32_t dwNetState, int32_t cbData, const char* pData){
		CServerClient* pSession = (CServerClient*)pNotify;
		pSession->m_smRecvBuf.AppendData(pData, cbData);
		if (pSession->m_smRecvBuf.GetDataLength() % sizeof(int) != 0)
			return BASIC_NET_OK;
		int nCount = pSession->m_smRecvBuf.GetDataLength() / sizeof(int);
		int* pIntData = (int*)pSession->m_smRecvBuf.GetDataBuffer();
		for (int i = 0; i < nCount; i++){
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
};

#define ADDRESS_S "0.0.0.0:8000"
#define ADDRESS_C "127.0.0.1:8000"

class CMultiClient : public CCommonClientSession{
	DefineCreateNetClient(CMultiClient)
protected:
	CMultiClient(){
		m_nAlreadySendNumber = 0;
		bind_connect(MakeFastFunction(this, &CMultiClient::bind_connectfunc));
		bind_rece(MakeFastFunction(this, &CMultiClient::OnReceiveVerify));
		bind_idle(MakeFastFunction(this, &CMultiClient::bind_idlefunc));
		bind_disconnect(MakeFastFunction(this, &CMultiClient::bind_disconnectfunc));
	}
	virtual ~CMultiClient(){}
public:
	int32_t bind_connectfunc(basiclib::CBasicSessionNetNotify* pNotify, uint32_t dwNetState){
		m_nAlreadySendNumber = 0;
		pNotify->Send((void*)strInfo.c_str(), strInfo.GetLength());
		return BASIC_NET_OK;
	}
	int32_t bind_idlefunc(basiclib::CBasicSessionNetNotify* pNotify, uint32_t dwNetState){
		Send((void*)&m_nAlreadySendNumber, sizeof(uint32_t));
		m_nAlreadySendNumber++;
		return BASIC_NET_OK;
	}
	int32_t bind_disconnectfunc(basiclib::CBasicSessionNetNotify* pNotify, uint32_t dwNetState){
		return BASIC_NET_OK;
	}

	int32_t OnReceive(basiclib::CBasicSessionNetNotify* pNotify, uint32_t dwNetState, int32_t cbData, const char* pData){
		return BASIC_NET_OK;
	}
	int32_t OnReceiveVerify(basiclib::CBasicSessionNetNotify* pNotify, uint32_t dwNetState, int32_t cbData, const char* pData){
		if(memcmp(strInfo.c_str(), pData, strInfo.GetLength()) == 0){
		}
		else{
			basiclib::BasicLogEvent(basiclib::DebugLevel_Info, "Client Verify fail!");
			Close();
			return BASIC_NET_OK;
		}
		bind_rece(MakeFastFunction(this, &CMultiClient::OnReceive));
		return BASIC_NET_HR_RET_HANDSHAKE;
	}

	int m_nAlreadySendNumber;
};

class CClient : public CCommonClientSession
{
	DefineCreateNetClient(CClient)
protected:
	CClient(){
		m_nSendNumber = 0;
		m_nAlreadySendNumber = 0;
		m_nReceiveNumber = 0;
		bind_connect(MakeFastFunction(this, &CClient::bind_connectfunc));
		bind_rece(MakeFastFunction(this, &CClient::OnReceiveVerify));
		bind_idle(MakeFastFunction(this, &CClient::bind_idlefunc));
		bind_disconnect(MakeFastFunction(this, &CClient::bind_disconnectfunc));
		m_bClose = false;
		m_nSendTimes = 0;
	}
	virtual ~CClient(){}
public:
	int32_t bind_connectfunc(basiclib::CBasicSessionNetNotify* pNotify, uint32_t dwNetState){
		m_nSendNumber = 0;
		m_nAlreadySendNumber = 0;
		m_nReceiveNumber = 0;
		pNotify->Send((void*)strInfo.c_str(), strInfo.GetLength());
		return BASIC_NET_OK;
	}
	int32_t bind_idlefunc(basiclib::CBasicSessionNetNotify* pNotify, uint32_t dwNetState){
		return BASIC_NET_OK;
	}
	int32_t bind_disconnectfunc(basiclib::CBasicSessionNetNotify* pNotify, uint32_t dwNetState){
		basiclib::CBasicString& strAddr = ((CBasicSessionNetClient*)pNotify)->GetConnectAddr();
		basiclib::BasicLogEventV(basiclib::DebugLevel_Info, "%x Client:DisConnect(%s)", this, strAddr.c_str());

		if(rand() % 100 > 70 && m_nSendTimes < 5){
			bind_rece(MakeFastFunction(this, &CClient::OnReceiveVerify));
			if(Connect(ADDRESS_C) != BASIC_NET_OK){
				basiclib::BasicLogEventV(basiclib::DebugLevel_Info, "connect error:%s\n", ADDRESS_C);
			}
		}
		else{
			m_bClose = true;
		}
		return BASIC_NET_OK;
	}
	
	void DoSend(int nCount = 0){
		if (m_nSendNumber <= m_nReceiveNumber){
			basiclib::BasicLogEventV(basiclib::DebugLevel_Info, "%x Client Send Finish:%d:%d", this, m_nAlreadySendNumber, m_nReceiveNumber);
			Close();
		}
		else{
			int nSendNumber = nCount == 0 ? ((rand() % 1000) + 100) : nCount;
			for (uint32_t i = 0; i < nSendNumber; i++){
				Send((void*)&m_nAlreadySendNumber, sizeof(uint32_t));
				m_nAlreadySendNumber++;
			}
		}
	}
	int32_t OnReceive(basiclib::CBasicSessionNetNotify* pNotify, uint32_t dwNetState, int32_t cbData, const char* pData){
		m_smRecvBuf.AppendData(pData, cbData);
		if (m_smRecvBuf.GetDataLength() % sizeof(int) != 0)
			return BASIC_NET_OK;
		int nCount = m_smRecvBuf.GetDataLength() / sizeof(int);
		int* pIntData = (int*)m_smRecvBuf.GetDataBuffer();
		for (int i = 0; i < nCount; i++){
			if (pIntData[i] != m_nReceiveNumber){
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
	int32_t OnReceiveVerify(basiclib::CBasicSessionNetNotify* pNotify, uint32_t dwNetState, int32_t cbData, const char* pData){
		if (memcmp(strInfo.c_str(), pData, cbData) == 0 && cbData == strInfo.GetLength()){
			m_nSendNumber = ((rand() % 65000) + 1) * 10;
			basiclib::BasicLogEventV(basiclib::DebugLevel_Info, "%x ClientSendNumber:%d\n", this, m_nSendNumber);
		}
        else{
            basiclib::BasicLogEvent(basiclib::DebugLevel_Info, "Client Verify fail!");
            Close();
            return BASIC_NET_OK;
        }

		bind_rece(MakeFastFunction(this, &CClient::OnReceive));
		DoSend(0);
		return BASIC_NET_HR_RET_HANDSHAKE;
	}
	
	int			m_nSendTimes;
	bool		m_bClose;
	uint32_t	m_nSendNumber;
	uint32_t	m_nAlreadySendNumber;
	uint32_t	m_nReceiveNumber;
	basiclib::CBasicSmartBuffer m_smRecvBuf;
};

//datacheck time
LONG	g_DataCheckClientThreadCount = 0;
THREAD_RETURN WorkerClientThread(void* arg){
	srand(time(NULL) + basiclib::BasicGetTickTime());
	CClient* pClient = CClient::CreateNetClient();
	if (pClient->Connect(ADDRESS_C) != BASIC_NET_OK){
		basiclib::BasicLogEventV(basiclib::DebugLevel_Info, "connect error:%s\n", ADDRESS_C);
	}
	while (!pClient->m_bClose){
		basiclib::BasicSleep(1000);
	}
	pClient->SafeDelete();
	basiclib::BasicLogEventV(basiclib::DebugLevel_Info, "WorkerClientThread Close");
	basiclib::BasicInterlockedIncrement(&g_DataCheckClientThreadCount);
	return 0;
}

bool g_bStartClose = false;
bool g_workServerClose = false;
THREAD_RETURN WorkerServerThread(void *arg){
	srand(time(NULL) + basiclib::BasicGetTickTime() + basiclib::BasicGetCurrentThreadId());
	basiclib::SetNetInitializeGetParamFunc([](const char* pParam)->basiclib::CBasicString{
		if (strcmp("NetThreadCount", pParam) == 0)
			return "4";
		return "";
	});
    int nCreateCount = 1;
	CServer* pServer = CServer::CreateNetServer();
	pServer->SetClientRecTimeout(30);
	int32_t lRet = pServer->StartServer(ADDRESS_S);
	int nIndex = 0;
	while (g_DataCheckClientThreadCount != nCreateCount){
		basiclib::BasicSleep(1000);
        if (nIndex < nCreateCount){
			DWORD dwThreadServerID = 0;
			basiclib::BasicCreateThread(WorkerClientThread, nullptr, &dwThreadServerID);
		}
		nIndex++;
	}
	
	vector<CMultiClient*> vtClient;
	nIndex = 0;
	while(!g_bStartClose){
		basiclib::BasicSleep(10);
		CMultiClient* pRet = CMultiClient::CreateNetClient();
		pRet->Connect(ADDRESS_C);
		vtClient.push_back(pRet);
		nIndex++;
		if(nIndex % 1000 == 999){
			basiclib::BasicLogEventV(basiclib::DebugLevel_Info, "ServerOnline %d", pServer->GetOnlineSessionCount());
		}
	}
	pServer->SafeDelete();
	for(auto& client : vtClient){
		client->SafeDelete();
	}
	g_workServerClose = true;
	basiclib::BasicLogEventV(basiclib::DebugLevel_Info, "WorkerServerThread Close");
	return 0;
}

void NetServerTest(){
	srand(time(NULL) + basiclib::BasicGetTickTime());
	basiclib::BasicLogEvent(basiclib::DebugLevel_Info, "StartServer\n");
	DWORD dwThreadServerID = 0;
	basiclib::BasicCreateThread(WorkerServerThread, nullptr, &dwThreadServerID);

	getchar();
	basiclib::BasicLogEvent(basiclib::DebugLevel_Info, "start end work\n");
	g_bStartClose = true;
	while(!g_workServerClose){
		basiclib::BasicSleep(1000);
	}
	basiclib::BasicLogEvent(basiclib::DebugLevel_Info, "finish end work\n");
	getchar();
}

