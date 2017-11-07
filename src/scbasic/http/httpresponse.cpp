#include "httpdefine.h"
#include "httpresponse.h"
#include <algorithm>
using namespace basiclib;

HttpResponse::HttpResponse(void)
{
	m_nStatus = HTTP_STATUS_OK;
	m_bZipped = false;
}

HttpResponse::~HttpResponse(void)
{
}

void HttpResponse::SetStatus(unsigned short uStatus)
{
	m_nStatus = uStatus;
}

void HttpResponse::SetVersion(const char* pszVersion)
{
	// 对于version信息不做校验，调用者自己必须保证格式正确。
	m_strVersion = pszVersion;
}

const char*	HttpResponse::GetVersion() const
{
	if (m_strVersion.empty())
	{
		return "1.1";
	}
	return m_strVersion.c_str();
}

void HttpResponse::AppendContent(const char* buf, size_t len)
{
	m_bufContent.AppendData(buf, len);
}

void HttpResponse::SetHeader(const char* key, const char* value, bool replace)
{
	// Header的头的每一个字母开头要大写。
	char_string strKey = key;
	for (size_t i = 0; i < strKey.length(); ++ i)
	{
		if (i == 0 || strKey[i - 1] == '-')
		{
			strKey[i] = toupper(strKey[i]);
		}
	}
	
	if (replace || m_Headers.find(strKey) != m_Headers.end())
		m_Headers[strKey] = value;
}

const char* HttpResponse::GetHeader(const char* key, const char* def) const
{
	HeaderContainer::const_iterator iter = m_Headers.find(key);
	if (iter != m_Headers.end())
		return (*iter).second.c_str();
	else
		return def;
}

class HttpResponse::WrapFillHeader
{
public:
	WrapFillHeader(CBasicSmartBuffer* buffer) : __buffer(buffer){}

	void operator()(const pair<char_string, char_string>& p)
	{
		HttpResponse::AppendHeader(__buffer, p.first.c_str(), p.second.c_str());
	}
protected:
	CBasicSmartBuffer*	__buffer;
};

void HttpResponse::GetHeaderData(CBasicSmartBuffer* buffer, long lContentLen) const
{
	buffer->AppendString("HTTP/");
	buffer->AppendString(GetVersion());
	buffer->AppendString(" ");
	buffer->AppendString(GetStatusString(m_nStatus));
	buffer->AppendString("\r\n");

	char buf[32];
	if (lContentLen != -1)
	{
		_snprintf(buf, sizeof(buf), "%ld", lContentLen);
	}
	else
	{
		_snprintf(buf, sizeof(buf), "%ld", m_bufContent.GetDataLength());
	}
	__CombineHeader(buffer, HTTP_CONTENT_LENGTH, buf);
	__CombineHeader(buffer, HTTP_DATE, GetTimeString(time(NULL), false).c_str());

	for_each(m_Headers.begin(), m_Headers.end(), WrapFillHeader(buffer));
	buffer->AppendData(CRLR, 2);
}

void HttpResponse::GetContent(CBasicSmartBuffer* buffer) const
{
	buffer->AppendData(m_bufContent.GetDataBuffer(), m_bufContent.GetDataLength());
}


void HttpResponse::AppendHeader(CBasicSmartBuffer* buffer, const char* key, const char* value)
{
	buffer->AppendData(key, strlen(key));
	buffer->AppendData(": ", 2);
	buffer->AppendData(value, strlen(value));
	buffer->AppendData(CRLR, 2);
}

const char*	HttpResponse::GetStatusString(unsigned short status)
{
	const HttpStatus* pBegin = HttpStatusTable;
	const HttpStatus* pEnd = HttpStatusTable + sizeof(HttpStatusTable)/sizeof(HttpStatus) - 1;
	const HttpStatus* pStatus = lower_bound(pBegin, pEnd, status);

	if (pStatus->m_pszStatusString != NULL && pStatus->m_nStatus == status)
	{
		return pStatus->m_pszStatusString;
	}
	return NULL;
}

void HttpResponse::__CombineHeader(CBasicSmartBuffer* buffer, const char* key, const char* value) const
{
	if (m_Headers.find(key) == m_Headers.end())
	{
		AppendHeader(buffer, key, value);
	}
}

char_string	HttpResponse::GetTimeString(time_t t, bool gmtime1)
{
	struct tm* pTtm;
#ifdef __WINDOWS
	if (gmtime1)
		pTtm = localtime(&t);
	else
		pTtm = gmtime(&t);
#else
	if (gmtime1)
		pTtm = localtime(&t);
	else
		pTtm = gmtime(&t);
#endif
	char buf[32];
	strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", pTtm);

	return char_string(buf);
}


bool HttpResponse::ZipOutput()
{
	if (m_bZipped)
		return true;

    //目前不支持压缩
	return false;
}

class HttpResponse::WrapFillResponse
{
public:
	WrapFillResponse(HttpResponse* pResponse) : m_pResponse(pResponse){}

	void operator()(const char* key, const char* value)
	{
		if (!IsStringEmpty(key) && !IsStringEmpty(value))
		{
			m_pResponse->SetHeader(key, value);
		}
	}
protected:
	HttpResponse* m_pResponse;
};

int HttpResponse::FromCacheBuffer(const char* pBuffer, size_t len)
{
	const char* pBody = strstr(pBuffer, "\r\n\r\n");
	if (pBody)
	{
		AppendContent(pBody + 4, len - (pBody - pBuffer) - 4);
	}

	char_string str(pBuffer, pBody - pBuffer);
	using namespace basiclib::__private;
	__ParseParamString_Aux((char*)str.c_str(), ":\n", WrapFillResponse(this), Parse_Trait<False, False, True>());

	SetStatus(HTTP_STATUS_OK);

	return HTTP_SUCC;
}

int	 HttpResponse::MakeCacheBuffer(CBasicSmartBuffer* pBuffer) const
{
	FillHeader(pBuffer, HTTP_CONTENT_TYPE);
	FillHeader(pBuffer, HTTP_ETAG);
	FillHeader(pBuffer, HTTP_EXPIRES);
	FillHeader(pBuffer, HTTP_LAST_MODIFIED);
	FillHeader(pBuffer, HTTP_CONTENT_ENCODING);
	FillHeader(pBuffer, HTTP_CONTENT_LANGUAGE);
	FillHeader(pBuffer, HTTP_AGE);
	pBuffer->AppendData(CRLR, 2);
	GetContent(pBuffer);
	return HTTP_SUCC;
}

void HttpResponse::FillHeader(CBasicSmartBuffer* pBuffer, const char* key) const
{
	HeaderContainer::const_iterator iter = m_Headers.find(key);
	if (iter != m_Headers.end())
	{
		AppendHeader(pBuffer, key, (*iter).second.c_str());
	}
}

unsigned short HttpResponse::GetStatus() const
{
	return m_nStatus;
}

bool HttpResponse::IsKeepAlive() const
{
	const char* pszConnection = GetHeader(HTTP_CONNECTION);
	return pszConnection && 0 == _stricmp(HTTP_VALUE_KEEP_ALIVE, pszConnection);
}

void HttpResponse::SetCookie(const char* key, const char* value, time_t expires, const char* path, const char* domain)
{
	char_string strCookie = GetHeader(HTTP_SET_COOKIE, Null_String_S);
	strCookie += key;
	strCookie += '=';
	strCookie += value;
	strCookie += ';';
	if (expires != 0)
	{
		strCookie += "expires=" + GetTimeString(expires, true) + ';';
	}
	if (!IsStringEmpty(path))
	{
		strCookie += "path=";
		strCookie += path;
		strCookie + ';';
	}
	if (!IsStringEmpty(domain))
	{
		strCookie += "domain=";
		strCookie += domain;
		strCookie += ';';
	}
	SetHeader(HTTP_SET_COOKIE, strCookie.c_str());
}