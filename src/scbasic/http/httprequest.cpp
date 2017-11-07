#include "../../inc/basic.h"
#include "httprequest.h"
#include "httpdefine.h"
using namespace basiclib;

HttpRequest::HttpRequest(void)
{
	m_pContext = NULL;
}

HttpRequest::~HttpRequest(void)
{
	for_each(m_conHandle.begin(), m_conHandle.end(), DeleteObjectAux());
	m_conHandle.clear();
}


void HttpRequest::MethodHandler(const char* method)
{
	m_strMethod = method;
	HeaderHandler(HTTP_HTTP_METHOD, method);
}
// 获取URI
// /index.php?a=b&c=d
void  HttpRequest::URIHandler(const char* URI)
{
	m_strURI = URI;
	HeaderHandler(HTTP_HTTP_URI, URI);
	ParseURI(URI);
}
// 获取协议
// HTTP/1.1
void HttpRequest::ProtocolHandler(const char* protocol)
{
	m_strProtocol = protocol;
	HeaderHandler(HTTP_HTTP_PROTOCOL, protocol);
}
// 得到http头
// Accept-Encoding: gzip, deflate
void HttpRequest::HeaderHandler(const char* key, const char* value)
{
	char_string v = Basic_URLDecode(value);
	if (Basic_IsUTF8Str(v.c_str(), v.length()))
		v = Basic_MultiStringToMultiString(v.c_str(), v.length(), CP_UTF8, CP_ACP);
	m_Headers[key] = v;
}
// 获得post的内容
void HttpRequest::PostDataHandler(const char* data, size_t len)
{
	m_bufBody.Release();
	m_bufBody.AppendBuffer(data, len);
}
// 一个请求包解析结束
void HttpRequest::RequestEndHandler()
{
	const char* pszCookie = GetHeaderValue(HTTP_COOKIE);
	if (!IsStringEmpty(pszCookie))
	{
		Basic_ParseParamString(pszCookie, "=;", IntoMapContainer_s<HeaderContainer>(m_Cookies));
	}
}

const char* HttpRequest::GetRequestData(const char *szkey) const
{

	return NULL;
}

void FilterFileInfo(char_string& strFile, char cSplite)
{
	if(__tcschr(strFile.c_str(), '.') < 0)
	{
		return;
	}
	CBasicStringArray ayItem;
	BasicSpliteString(strFile.c_str(), cSplite, basiclib::IntoContainer_s<basiclib::CBasicStringArray>(ayItem));
	int nSize = ayItem.GetSize();
	for (int i = 0;i < nSize;i++)
	{
		if (ayItem[i] == "..")
		{
			ayItem[i].Empty();
			if (i > 0)
			{
				ayItem[i - 1].Empty();
			}
		}
		else if (ayItem[i] == ".")
		{
			ayItem[i].Empty();
		}
	}
	strFile = '/';
	strFile += Basic_CombineString(cSplite, ayItem.begin(), ayItem.end());
	
}

void HttpRequest::ParseURI(const char* URI)
{
	const char* pszArg = __tcschr(URI, '?');
	if (pszArg != NULL)
	{
		m_strArgs = pszArg;
		Basic_ParseParamString(pszArg + 1, "=&", FillURIParam<HeaderContainer>(m_Params));
		CHttpParser::DecodeString(char_string(URI, pszArg - URI).c_str(), m_strFile);
	}
	else
	{
		CHttpParser::DecodeString(URI, m_strFile);
	}

	replace(m_strFile.begin(), m_strFile.end(), '\\', '/');
	FilterFileInfo(m_strFile, '/');
}

bool HttpRequest::GetHeaderValue(const char* key, char_string& value) const
{
	char_string strKey(key);
	strutil::makelower(strKey);

	HeaderContainer::const_iterator iter = m_Headers.find(strKey);
	if (iter != m_Headers.end())
	{
		value = (*iter).second;
		return true;
	}
	return false;
}

const char*	HttpRequest::GetHeaderValue(const char* key) const
{
	char_string strKey(key);
	strutil::makelower(strKey);

	HeaderContainer::const_iterator iter = m_Headers.find(strKey);
	if (iter != m_Headers.end())
	{
		return (*iter).second.c_str();
	}
	return NULL;
}

bool HttpRequest::GetHeaderValue(const char* key, basiclib::CTime& value) const
{
	char_string strValue;
	if (GetHeaderValue(key, strValue))
	{
		int nPos = strValue.find(';');
		if (nPos != -1)
		{
			strValue = strValue.substr(0, nPos);
		}

		time_t t = strtotime(strValue.c_str(), strValue.length());
		if (-1 != t)
		{
			value = t;
			return true;
		}
	}
	return false;
}

bool HttpRequest::GetHeaderValue(const char* key, long& value) const
{
	char_string strValue;
	if (GetHeaderValue(key, strValue))
	{
		value = atol(strValue.c_str());
		return true;
	}
	return false;
}

const char_string&	HttpRequest::GetMethod() const
{
	return m_strMethod;
}

const char_string&	HttpRequest::GetURI() const
{
	return m_strURI;
}

const char_string&	HttpRequest::GetProtocol() const
{
	return m_strProtocol;
}

const char_string&	HttpRequest::GetFileName() const
{
	return m_strFile;
}

const char* HttpRequest::GetParamValue(const char* key, const char* pDefault) const
{
	HeaderContainer::const_iterator iter = m_Params.find(key);
	if (iter != m_Params.end())
	{
		return (*iter).second.c_str();
	}
	return pDefault;
}

const char* HttpRequest::GetContentData() const
{
	return (const char*)m_bufBody.GetBuffer();
}

unsigned long HttpRequest::GetContentLength() const
{
	return m_bufBody.GetLength();
}

void HttpRequest::Reset()
{	//重置
	m_strMethod	= "";
	m_strURI	= "";
	m_strProtocol	= "";
	m_strFile	= "";
	m_strArgs	= "";

	m_Params.clear();
	m_Headers.clear();
	m_bufBody.Release();
	m_pContext = NULL;
}

void HttpRequest::SetContext(void* pContext)
{
	m_pContext = pContext;
}

void* HttpRequest::GetContext() const
{
	return m_pContext;
}

BOOL HttpRequest::IsKeepAlive() const
{
	const char* lpKeepAlive = GetHeaderValue(HTTP_CONNECTION);
	if (!IsStringEmpty(lpKeepAlive))
	{
		return (0 == _stricmp(lpKeepAlive, HTTP_KEEP_ALIVE));
	}
	else
	{
		return (0 == _stricmp(m_strProtocol.c_str(), "HTTP/1.1"));	// HTTP/1.1默认支持Keep-Alive
	}
	return FALSE;
}

BOOL HttpRequest::IsSupportGZip() const
{
	const char* lpszAcceptEncoding = GetHeaderValue(HTTP_ACCEPT_ENCODING);
	return !IsStringEmpty(lpszAcceptEncoding) && (NULL != __tcsstr(lpszAcceptEncoding, "gzip"));
		
}

void HttpRequest::SetPeerAddr(const char* addr)
{
	m_strPeerAddr = addr;
}
basiclib::CBasicString& HttpRequest::GetRefPeerAddr()
{
	return m_strPeerAddr;
}

const char* HttpRequest::GetPeerAddr() const
{
	return m_strPeerAddr.c_str();
}

void HttpRequest::SetFileName(const char* file)
{
	m_strFile = file;
}

bool HttpRequest::CanCache() const
{
	if (m_bufBody.GetLength() == 0)
	{
		const char* cacheControl = GetHeaderValue(HTTP_CACHE_CONTROL);
		if (NULL == cacheControl || 0 != strcmp(cacheControl, "no-cache"))
			return true;
	}
	return false;
}

const char* HttpRequest::GetCookie(const char* key) const
{
	char_string strKey(key);
	strutil::makelower(strKey);

	HeaderContainer::const_iterator iter = m_Cookies.find(strKey);
	if (iter != m_Cookies.end())
	{
		return (*iter).second.c_str();
	}
	return NULL;
}

void HttpRequest::AddContextHandle(const char* szKey, CContextHandle* pHandle)
{
	if(!IsStringEmpty(szKey) && pHandle)
	{
		m_conHandle[szKey] = pHandle;
	}
}

CContextHandle* HttpRequest::FindHandle(const char* szKey) const
{
	if (IsStringEmpty(szKey))
		return NULL;

	ContextHandleContainer::const_iterator iter = m_conHandle.find(szKey);
	if(iter != m_conHandle.end())
	{
		return (*iter).second;
	}
	return NULL;
}


void HttpRequest::RemoveContextHandle(const char* szKey)
{
	CContextHandle* pHandle = FindHandle(szKey);
	if(pHandle)
	{
		delete pHandle;
		m_conHandle.erase(szKey);
	}
}

long HttpRequest::HandleContext(const char* szKey, void* pContext, CBasicSmartBuffer& buffer) const
{	
	CContextHandle* pHandle = FindHandle(szKey);
	if(pHandle)
	{
		return pHandle->HandleContext(pContext, buffer);
	}
	return -1;
}