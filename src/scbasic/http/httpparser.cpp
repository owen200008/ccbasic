#include "httpparser.h"
using namespace basiclib;

const char*	key_content_length = "content-length";

CHttpParser::CHttpParser()
{
}

CHttpParser::~CHttpParser()
{
}

const size_t MAX_URI = 4096;

int	 CHttpParser::Parse(const char* buffer, size_t len, size_t& remain, IHttpParseHandler* pHandler)
{
#define SET_STATUS(a)	{__context.m_eStatus = (a); token = p + 1;}
#define ISBLANK(a)		((a)==' ' || (a)=='\t' || (a)=='\r' || (a)=='\n')

	size_t streamlen = len;
	size_t nLastParseLen = __context.m_bufTemp.GetDataLength();

	if (buffer && len)
		__context.m_bufTemp.AppendData(buffer, len);
	char* token = __context.m_bufTemp.GetDataBuffer();
	
	char* p = NULL;
	if (buffer && len)
	{
		p = token + nLastParseLen;
	}
	else
	{
		p = token;
		len = __context.m_bufTemp.GetDataLength();
	}

	for(;len > 0 && __context.m_eStatus != PS_Body && __context.m_eStatus != PS_End; ++p, --len)
	{
		if (token == p && *p != '\n' && ISBLANK(*p))
		{
			++ token;
			continue;
		}

		switch(__context.m_eStatus)
		{
		case PS_Method:
			if (p - token > 10 || (*p) == '\n')
			{
				return HTTP_ERROR_SYNTAX;
			}
			else if (ISBLANK(*p))
			{
				*p = '\0';
				if (_stricmp(token, "GET") != 0
					&& _stricmp(token, "POST") != 0)
				{
					return HTTP_ERROR_METHOD;
				}
				pHandler->MethodHandler(token);
				SET_STATUS(PS_URI);
			}
			break;
		case PS_URI:
			if ((*p) == '\n')
			{
				return HTTP_ERROR_SYNTAX;
			}
			else if (ISBLANK(*(unsigned char*)p))
			{
				*p = '\0';
				pHandler->URIHandler(token);
				SET_STATUS(PS_Protocol);
			}
			else if (p - token > MAX_URI)	// 留4K
			{
				return HTTP_ERROR_URI_TOOLONG;
			}
			break;
		case PS_Protocol:
			if (*p == '\n')
			{
				*p = '\0';
				pHandler->ProtocolHandler(Basic_RTrim(token));
				SET_STATUS(PS_Header_Key);
			}
			break;
		case PS_Header_Key:
			switch(*p)
			{
				case ':':
					__context.m_strKey = Basic_RTrim(token, p - token);
					SET_STATUS(PS_Header_Value);
					break;
				case '\n':
					if (token == p)
					{// 空行，结束了
						if (__context.m_nBodyLen == 0)
						{
							SET_STATUS(PS_End);
						}
						else
						{
							SET_STATUS(PS_Body);
						}
					}
					else
					{
						SET_STATUS(PS_Header_Key);
					}
					break;
				default:
					if (*p >= 'A' && *p <= 'Z')
					{
						*p = *p + 32;	// make lower; 'a'-'A'==32
					}
					break;
			}
			break;
		case PS_Header_Value:
			switch(*p)
			{
			case '\n':
				*p = '\0';
				pHandler->HeaderHandler(__context.m_strKey.c_str(), Basic_RTrim(token, p - token));
				if (__context.m_strKey == key_content_length)
					__context.m_nBodyLen = atol(token);
				SET_STATUS(PS_Header_Key);
			}
			break;
		}
	}
	len += (p - token);
	// mark a rule: 以下不能对p进行操作.

	if (__context.m_eStatus == PS_Body)
	{
		if (len >= __context.m_nBodyLen)
		{	// 剩下的长度足够body了。
			pHandler->PostDataHandler(token, __context.m_nBodyLen);
			len -= __context.m_nBodyLen;
			token += __context.m_nBodyLen;
			__context.m_eStatus = PS_End;
		}
	}
	
	if (len > 0)
		memmove(__context.m_bufTemp.GetDataBuffer(), token, len);
	__context.m_bufTemp.SetDataLength(len);

	if (__context.m_eStatus	== PS_End)
	{
		pHandler->RequestEndHandler();
		__context.Reset();
		remain = len;
		return (len > 0) ? HTTP_ERROR_NEWREQUEST : HTTP_ERROR_FINISH;
	}
	else
	{
		return HTTP_ERROR_WANTMORE;
	}
}

void CHttpParser::DecodeString(const char* strIn, char_string& strOut)
{
	strOut = Basic_URLDecode(strIn);
	if (Basic_IsUTF8Str(strOut.c_str(), strOut.length()))
		strOut = Basic_MultiStringToMultiString(strOut.c_str(), strOut.length(), CP_UTF8, CP_ACP);
}
