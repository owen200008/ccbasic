#ifndef _INC_HTTPSESSION_H
#define _INC_HTTPSESSION_H

/////////////////////////////////////////////////////////////////////////////////////////////////
#include "../commu/servertemplate.h"

class HttpRequest;
class HttpResponse;
class CHttpParser;
class CHttpSession;

typedef basiclib::CBasicRefPtr<CHttpSession>		RefHttpSession;
typedef fastdelegate::FastDelegate3<RefHttpSession, HttpRequest*, HttpResponse&, long> OnHttpAskFunc;
class CHttpSession : public CNetServerControlClient
{
public:
	static CHttpSession* CreateHttpClient(Net_UInt nSessionID, CRefNetServerControl pServer){ return new CHttpSession(nSessionID, pServer); }
protected:
	CHttpSession(Net_UInt nSessionID, CRefNetServerControl pServer) : CNetServerControlClient(nSessionID, pServer){
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

class CHttpSessionServer : public CNetServerControl
{
public:
	static CHttpSessionServer* CreateHttpServer(Net_UInt nSessionID){ return new CHttpSessionServer(nSessionID); }
protected:
	CHttpSessionServer(Net_UInt nSessionID);
	virtual ~CHttpSessionServer();

public:
	void bind_onhttpask(OnHttpAskFunc& func)
	{
		m_funcOnHttpAsk = func;
	}
protected:
	virtual basiclib::CBasicSessionNetClient* ConstructSession(Net_UInt nSessionID);
protected:
	OnHttpAskFunc	m_funcOnHttpAsk;
};

/*
class HttpResponseHandler : public ResponseHandler
{
public:
	HttpResponseHandler(RefHttpSession pSession)
		:  m_pSession(pSession){}

	virtual int HandleResponse(long lStatus, HttpResponse* pResponse)
	{
		if (!m_pSession->IsConnected())
		{
			return HTTP_SUCC;
		}
        if (lStatus == HTTP_SUCC)
        {
            basiclib::CBasicSmartBuffer buf;
            pResponse->GetHeaderData(&buf);
            pResponse->GetContent(&buf);
            if (!buf.IsEmpty())
            {
                m_pSession->Send(buf.GetDataBuffer(), buf.GetDataLength());
            }
        }
        else
        {
            pResponse->SetStatus((unsigned short)lStatus);
        }
		m_pSession->Close();
		return HTTP_SUCC;
	}

	virtual void Release()
	{
		delete this;
	}

protected:
	RefHttpSession	m_pSession;
};*/

#endif //_INC_HTTPSESSION_H



