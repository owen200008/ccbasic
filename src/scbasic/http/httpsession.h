#ifndef _INC_HTTPSESSION_H
#define _INC_HTTPSESSION_H

/////////////////////////////////////////////////////////////////////////////////////////////////
#include "../commu/servertemplate.h"

#pragma warning (push)
#pragma warning (disable: 4251)
#pragma warning (disable: 4275)

class HttpRequest;
class HttpResponse;
class CHttpParser;
class CHttpSession;

typedef basiclib::CBasicRefPtr<CHttpSession>		RefHttpSession;
typedef fastdelegate::FastDelegate3<RefHttpSession, HttpRequest*, HttpResponse&, long> OnHttpAskFunc;
class _SCBASIC_DLL_API CHttpSession : public CNetServerControlSession
{
	DefineCreateNetServerSessionWithServer(CHttpSession)
public:
    void AsynSendResponse(HttpRequest*& pRequest, HttpResponse& response, basiclib::CBasicSmartBuffer& smBuf);
protected:
	CHttpSession(){
		m_pRequest = NULL;
		m_pParser = NULL;
		m_tLastRequest = time(NULL);
	}
	virtual ~CHttpSession();
	virtual Net_Int OnReceive(Net_UInt dwNetCode, const char *pszData, Net_Int cbData);
protected:
	CHttpParser*	GetParser();
protected:
	HttpRequest*												m_pRequest;
	CHttpParser*												m_pParser;
	time_t														m_tLastRequest;
	OnHttpAskFunc												m_funcOnHttpAsk;
	basiclib::CBasicSmartBuffer									m_smCacheBuf;
	friend class CHttpSessionServer;
};

class _SCBASIC_DLL_API CHttpSessionServer : public CNetServerControl
{
	DefineCreateNetServerDefault(CHttpSessionServer)
public:
	void bind_onhttpask(OnHttpAskFunc& func)
	{
		m_funcOnHttpAsk = func;
	}
protected:
	virtual basiclib::CBasicSessionNetServerSession* ConstructSession(uint32_t nSessionID);
protected:
	OnHttpAskFunc	m_funcOnHttpAsk;
};

#pragma warning (pop)

#endif //_INC_HTTPSESSION_H



