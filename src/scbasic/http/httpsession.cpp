#include "../../inc/basic.h"
#include "httpdefine.h"
#include "httprequest.h"
#include "httpresponse.h"
#include "httpparser.h"
#include "httpsession.h"

using namespace basiclib;

CHttpSession::~CHttpSession()
{
	if (m_pRequest)
	{
		delete m_pRequest;
		m_pRequest = NULL;
	}
	if (m_pParser)
	{
		delete m_pParser;
		m_pParser = NULL;
	}
}

CHttpParser* CHttpSession::GetParser()
{
	if (m_pParser == NULL)
		m_pParser = new CHttpParser();
	return m_pParser;
}

Net_Int CHttpSession::OnReceive(Net_UInt dwNetCode, const char *pszData, Net_Int cbData)
{
	m_tLastRequest = time(NULL);
	CHttpParser* pParser = GetParser();
	int ret = 0;
	do
	{
		if (NULL == m_pRequest)
			m_pRequest = new HttpRequest();
		size_t remain = 0;
		ret = pParser->Parse(pszData, cbData, remain, m_pRequest);
		if (ret == HTTP_ERROR_NEWREQUEST || ret == HTTP_ERROR_FINISH)
		{
			HttpResponse response;
			GetNetAddress(m_pRequest->GetRefPeerAddr());
			if (m_funcOnHttpAsk)
			{
				long lRet = m_funcOnHttpAsk(this, m_pRequest, response);
				if (lRet == HTTP_ASYNC)
				{
					m_pRequest = NULL;
				}
				else if (lRet == HTTP_SUCC)
				{
					response.GetHeaderData(&m_smCacheBuf);
					response.GetContent(&m_smCacheBuf);
					Send(m_smCacheBuf.GetDataBuffer(), m_smCacheBuf.GetDataLength());
					m_smCacheBuf.SetDataLength(0);
				}
				else if (lRet == HTTP_INTERNAL_ERROR)
				{
					ret = HTTP_ERROR_GENERIC;
					Close();
				}
			}
			if (m_pRequest)
			{
				delete m_pRequest;
				m_pRequest = NULL;
			}

			if (ret == HTTP_ERROR_NEWREQUEST)
			{
				ASSERT(cbData > (long)remain);
				pszData += (cbData - (long)remain);
				cbData = remain;
			}
		}
		else if (ret < 0)
		{
			Close();
			return BASIC_NET_GENERIC_ERROR;
		}
	} while (ret == HTTP_ERROR_NEWREQUEST);
	return BASIC_NET_OK;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
CHttpSessionServer::CHttpSessionServer(Net_UInt nSessionID) : CNetServerControl(nSessionID)
{

}

CHttpSessionServer::~CHttpSessionServer()
{

}

basiclib::CBasicSessionNetClient* CHttpSessionServer::ConstructSession(Net_UInt nSessionID)
{
	CHttpSession* pNotify = CHttpSession::CreateHttpClient(nSessionID, this);
	pNotify->m_funcOnHttpAsk = m_funcOnHttpAsk;
	return pNotify;
}
