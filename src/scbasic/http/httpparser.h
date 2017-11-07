#pragma once
#ifndef INC_CIFOX_HTTPPARSER_H__
#define INC_CIFOX_HTTPPARSER_H__

#include "../../inc/basic.h"
#include "../scbasic_head.h"

#pragma warning (push)
#pragma warning (disable: 4251)
#pragma warning (disable: 4275)

class _SCBASIC_DLL_API IHttpParseHandler : public basiclib::CBasicObject
{
public:
	// 获取请求方式.
	// GET,POST等
	virtual void	MethodHandler(const char* method) = 0;
	// 获取URI
	// /index.php?a=b&c=d
	virtual void	URIHandler(const char* URI) = 0;
	// 获取协议
	// HTTP/1.1
	virtual void	ProtocolHandler(const char* protocol) = 0;
	// 得到http头
	// Accept-Encoding: gzip, deflate
	virtual void	HeaderHandler(const char* key, const char* value) = 0;
	// 获得post的内容
	virtual void	PostDataHandler(const char* data, size_t len) = 0;
	// 一个请求包解析结束
	virtual void	RequestEndHandler() = 0;
};


const int	HTTP_ERROR_WANTMORE			= 1;	// 解析未结束，需要数据
const int	HTTP_ERROR_NEWREQUEST		= 2;	// 新的请求到来
const int	HTTP_ERROR_FINISH			= 0;	// 正常
const int	HTTP_ERROR_NODATA			= -1;	// 无数据
const int	HTTP_ERROR_PROTOCOL			= -2;	// 协议不支持
const int	HTTP_ERROR_URI_TOOLONG		= -3;	// URI超出最大限制
const int	HTTP_ERROR_HEADER_TOOLONG	= -4;	// http头长度超出最大限制
const int	HTTP_ERROR_SYNTAX			= -5;	// 格式错误
const int	HTTP_ERROR_METHOD			= -6;	// method不支持
const int	HTTP_ERROR_GENERIC			= -99;	// 错误

enum HttpParseStatus
{
	PS_Method,
	PS_URI,
	PS_Protocol,
	PS_Header_Key,
	PS_Header_Value,
	PS_Body,
	PS_End
};

struct _SCBASIC_DLL_API HttpParseContext
{
	HttpParseStatus	m_eStatus;
	basiclib::CBasicSmartBuffer	m_bufTemp;	// 临时buffer，用于存储上个请求包未解析完部分
	basiclib::char_string		m_strKey;	// 解析header时的key值。
	size_t			m_nBodyLen;
	
	HttpParseContext()
	{
		m_eStatus = PS_Method;
		m_nBodyLen = 0;
	}
	void Reset()
	{
		m_eStatus = PS_Method;
		m_nBodyLen = 0;
//		m_bufTemp.SetDataLength(0);
		m_strKey.empty();
	}
};

// http数据流的解析器
class _SCBASIC_DLL_API CHttpParser : public basiclib::CBasicObject
{
public:
	CHttpParser();
	~CHttpParser();

	int		Parse(const char* buffer, size_t len, size_t& remain, IHttpParseHandler* pHandler);
	static void DecodeString(const char* strIn, basiclib::char_string& strOut);

protected:
	HttpParseContext	__context;
};



/*!
解析URI信息，把key和value进行uri解码和utf8解码，放到map容器中。
*/
template<class MapContainer>
class _SCBASIC_DLL_API FillURIParam
{
public:
	FillURIParam(MapContainer& container) : __container(container) {}

	void operator()(const char* key, const char* value)
	{
		tstring_s k, v;
		CHttpParser::DecodeString(key, k);
		CHttpParser::DecodeString(value, v);
		__container[k.c_str()] = v.c_str();
	}
protected:
	MapContainer&	__container;
};
#pragma warning (pop)


#endif //INC_CIFOX_HTTPPARSER_H__



