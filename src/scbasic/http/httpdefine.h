#pragma once
#ifndef INC_CIFOX_HTTPDEFINE_H__
#define INC_CIFOX_HTTPDEFINE_H__

#include "../../inc/basic.h"

const char	HTTP_ACCEPT[]				= ("Accept");
const char	HTTP_ACCEPT_ENCODING[]		= ("Accept-Encoding");
const char	HTTP_ACCEPT_LANGUAGE[]		= ("Accept-Language");
const char	HTTP_ACCEPT_CHARSET[]		= ("Accept-Charset");
const char	HTTP_ACCEPT_RANGES[]		= ("Accept-Ranges");
const char	HTTP_AGE[]					= ("Age");
const char	HTTP_ALLOWED[]				= ("Allowed");
const char	HTTP_CACHE_CONTROL[]		= ("Cache-Control");
const char	HTTP_CONNECTION[]			= ("Connection");
const char	HTTP_CONTENT_ENCODING[]		= ("Content-Encoding");
const char	HTTP_CONTENT_DISPOSITION[]	= ("Content-Disposition");
const char	HTTP_CONTENT_LANGUAGE[]		= ("Content-Language");
const char	HTTP_CONTENT_LENGTH[]		= ("Content-Length");
const char	HTTP_CONTENT_RANGE[]		= ("Content-Range");
const char	HTTP_CONTENT_TRANSFER_ENCODING[] = ("Content-Transfer-Encoding");
const char	HTTP_CONTENT_TYPE[]			= ("Content-Type");
const char	HTTP_COST[]					= ("Cost");
const char	HTTP_COOKIE[]				= ("Cookie");
const char	HTTP_DATE[]					= ("Date");
const char	HTTP_DERIVED_FROM[]			= ("Derived-From");
const char	HTTP_ETAG[]					= ("Etag");
const char	HTTP_EXPIRES[]				= ("Expires");
const char	HTTP_HOST[]					= ("Host");
const char	HTTP_IF_MODIFIED_SINCE[]	= ("If-Modified-Since");
const char	HTTP_IF_NONE_MATCH[]		= ("If-None-Match");
const char	HTTP_LAST_MODIFIED[]		= ("Last-Modified");
const char	HTTP_LINK[]					= ("Link");
const char	HTTP_LOCATION[]				= ("Location");
const char	HTTP_KEEP_ALIVE[]			= ("Keep-Alive");
const char	HTTP_MESSAGE_ID[]			= ("Message-Id");
const char	HTTP_PUBLIC[]				= ("Public");
const char	HTTP_RANGE[]				= ("Range");
const char	HTTP_REFERER[]				= ("Referer");
const char	HTTP_SERVER[]				= ("Server");
const char	HTTP_SET_COOKIE[]			= ("Set-Cookie");
const char	HTTP_TITLE[]				= ("Title");
const char	HTTP_URI[]					= ("URI");
const char	HTTP_USER_AGENT[]			= ("User-Agent");
const char	HTTP_Version[]				= ("Version");
const char	HTTP_VIA[]					= ("Via");
const char	HTTP_X_CACHE[]				= ("X-Cache");
const char	HTTP_ACCESS_CONTROL_ALLOW_ORIGIN[]	= ("Access-Control-Allow-Origin");

const char	HTTP_VALUE_KEEP_ALIVE[]	= ("keep-alive");
const char	HTTP_VALUE_CLOSE[]		= ("close");

const char	HTTP_HTTP_METHOD[]		= ("_http_method");
const char	HTTP_HTTP_PROTOCOL[]	= ("_http_protocol");
const char	HTTP_HTTP_URI[]			= ("_http_uri");

const char	ENV_SERVER_NAME[]		= ("server_name");
const char	ENV_SERVER_ADDR[]		= ("server_addr");
const char	ENV_SERVER_PORT[]		= ("server_port");
const char	ENV_SERVER_ROOT[]		= ("server_root");
const char	ENV_DOCUMENT_ROOT[]		= ("document_root");
const char	ENV_EXTENSION_ROOT[]	= ("extension_root");
const char	ENV_CONNECTION[]		= ("connection");
const char	ENV_KEEP_ALIVE[]		= ("keep-alive");
const char	ENV_CONTENT_TYPE[]		= ("content-type");
const char	ENV_CACHE_EXPIRE[]		= ("cache-expire");

const char	DEFAULT_MIMETYPE[]		= ("text/html");
const char	CRLR[] = ("\r\n");

// HTTP请求方式
const char	HTTP_METHOD_GET[]		=	("GET");
const char	HTTP_METHOD_POST[]		=	("POST");

// config key
const char  EL_ACCESSLOG[]			= ("accesslog");
const char  EL_FORMAT[]			= ("format");
const char	EL_LOGSIZE[]			= ("logsize");
const char	EL_RECORDTYPE[]			= ("recordtype");

// http status table
// 1XX
const unsigned short HTTP_STATUS_CONTINUE							= 100;
const unsigned short HTTP_STATUS_SWITCHING_PROTOCOL					= 101;
const unsigned short HTTP_STATUS_PROCESSING							= 102;
// 2xx succ
const unsigned short HTTP_STATUS_OK									= 200;
const unsigned short HTTP_STATUS_CREATED							= 201;
const unsigned short HTTP_STATUS_ACCEPTED							= 202;
const unsigned short HTTP_STATUS_NON_AUTH_INFO						= 203;
const unsigned short HTTP_STATUS_NO_CONTENT							= 204;
const unsigned short HTTP_STATUS_RESET_CONTENT						= 205;
const unsigned short HTTP_STATUS_PARTIAL_CONTENT					= 206;
const unsigned short HTTP_STATUS_MULTI_STATUS						= 207;
const unsigned short HTTP_STATUS_IM_USED							= 226;
// 3xx redirection
const unsigned short HTTP_MULTIPLE_CHOICES							= 300;
const unsigned short HTTP_STATUS_MOVED								= 301;
const unsigned short HTTP_STATUS_FOUND								= 302;
const unsigned short HTTP_STATUS_SEE_OTHER							= 303;
const unsigned short HTTP_STATUS_NOT_MODIFIED						= 304;
const unsigned short HTTP_STATUS_USE_PROXY							= 305;
const unsigned short HTTP_STATUS_RESERVED							= 306;
const unsigned short HTTP_STATUS_REDIRECT							= 307;
// 4xx error
const unsigned short HTTP_STATUS_BAD_REQUEST						= 400;
const unsigned short HTTP_STATUS_UNAUTHORIZED						= 401;
const unsigned short HTTP_STATUS_PAYMENTREQUIRED					= 402;
const unsigned short HTTP_STATUS_FORBIDDEN							= 403;
const unsigned short HTTP_STATUS_NOT_FOUND							= 404;
const unsigned short HTTP_STATUS_METHOD_NOT_ALLOWED					= 405;
const unsigned short HTTP_STATUS_NOT_ACCEPTABLE						= 406;
const unsigned short HTTP_STATUS_PROXY_AUTHENTICATION_REQUIRED		= 407;
const unsigned short HTTP_STATUS_REQUEST_TIME_OUT					= 408;
const unsigned short HTTP_STATUS_CONFLICT							= 409;
const unsigned short HTTP_STATUS_GONE								= 410;
const unsigned short HTTP_STATUS_LENGTH_REQUIRED					= 411;
const unsigned short HTTP_STATUS_PRECONDITION_FAILED				= 412;
const unsigned short HTTP_STATUS_REQUEST_ENTITY_TOO_LONG			= 413;
const unsigned short HTTP_STATUS_REQUEST_URI_TOO_LONG				= 414;
const unsigned short HTTP_STATUS_UNSUPPORT_MEDIA_TYPE				= 415;
const unsigned short HTTP_STATUS_REQUESTED_RANGE_NOT_SATISFIABLE	= 416;
const unsigned short HTTP_STATUS_EXPECTATION_FAILED					= 417;
const unsigned short HTTP_STATUS_UNPROCESSABLE_ENTITY				= 422;
const unsigned short HTTP_STATUS_LOCKED								= 423;
const unsigned short HTTP_STATUS_FAILED_DEPENDENCY					= 424;
const unsigned short HTTP_STATUS_UPGRADE_REQUIRED					= 426;
// 5xx error
const unsigned short HTTP_STATUS_INTERNAL_SERVER_ERROR				= 500;
const unsigned short HTTP_STATUS_NOT_IMPLEMENTED					= 501;
const unsigned short HTTP_STATUS_BAD_GATEWAY						= 502;
const unsigned short HTTP_STATUS_SERVICE_UNAVAILABLE				= 503;
const unsigned short HTTP_STATUS_GATEWAY_TIMEOUT					= 504;
const unsigned short HTTP_STATUS_VERSION_NOT_SUPPORTED				= 505;
const unsigned short HTTP_STATUS_VARIANT_ALSO_NEGOTIATES			= 506;
const unsigned short HTTP_STATUS_INSUFFICIENT_STORAGE				= 507;
const unsigned short HTTP_STATUS_NOT_EXTENED						= 510;

struct HttpStatus
{
	unsigned short	m_nStatus;
	const char*		m_pszStatusString;
};

inline bool operator<(const HttpStatus& lhs, unsigned short rhs)
{
	return lhs.m_nStatus < rhs;
}

inline bool operator<(unsigned short lhs, const HttpStatus& rhs)
{
	return lhs < rhs.m_nStatus;
}
inline bool operator<(const HttpStatus& lhs, const HttpStatus& rhs)
{
	return lhs.m_nStatus < rhs.m_nStatus;
}

const HttpStatus HttpStatusTable[] = 
{
	{HTTP_STATUS_CONTINUE,							("100 Continue")},
	{HTTP_STATUS_SWITCHING_PROTOCOL,				("101 Switching Protocols")},
	{HTTP_STATUS_PROCESSING,						("102 Processing")},                
	{HTTP_STATUS_OK,								("200 OK")},
	{HTTP_STATUS_CREATED,							("201 Created")},
	{HTTP_STATUS_ACCEPTED,							("202 Accepted")},
	{HTTP_STATUS_NON_AUTH_INFO,						("203 Non-Authoritative Information")},
	{HTTP_STATUS_NO_CONTENT,						("204 No Content")},
	{HTTP_STATUS_RESET_CONTENT,						("205 Reset Content")},
	{HTTP_STATUS_PARTIAL_CONTENT,					("206 Partial Content")},
	{HTTP_STATUS_MULTI_STATUS,						("207 Multi-Status")},
	{HTTP_STATUS_IM_USED,							("226 IM Used")},
	{HTTP_MULTIPLE_CHOICES,							("300 Multiple Choices")},
	{HTTP_STATUS_MOVED,								("301 Moved Permanently")},
	{HTTP_STATUS_FOUND,								("302 Found")},
	{HTTP_STATUS_SEE_OTHER,							("303 See Other")},
	{HTTP_STATUS_NOT_MODIFIED,						("304 Not Modified")},
	{HTTP_STATUS_USE_PROXY,							("305 Use Proxy")},
	{HTTP_STATUS_RESERVED,							("306 Reserved")},
	{HTTP_STATUS_REDIRECT,							("307 Temporary Redirect")},
	{HTTP_STATUS_BAD_REQUEST,						("400 Bad Request")},
	{HTTP_STATUS_UNAUTHORIZED,						("401 Unauthorized")},
	{HTTP_STATUS_PAYMENTREQUIRED,					("402 Payment Required")},
	{HTTP_STATUS_FORBIDDEN,							("403 Forbidden")},
	{HTTP_STATUS_NOT_FOUND,							("404 Not Found")},
	{HTTP_STATUS_METHOD_NOT_ALLOWED,				("405 Method Not Allowed")},
	{HTTP_STATUS_NOT_ACCEPTABLE,					("406 Not Acceptable")},
	{HTTP_STATUS_PROXY_AUTHENTICATION_REQUIRED,		("407 Proxy Authentication Required")},
	{HTTP_STATUS_REQUEST_TIME_OUT,					("408 Request Timeout")},
	{HTTP_STATUS_CONFLICT,							("409 Conflict")},
	{HTTP_STATUS_GONE,								("410 Gone")},
	{HTTP_STATUS_LENGTH_REQUIRED,					("411 Length Required")},
	{HTTP_STATUS_PRECONDITION_FAILED,				("412 Precondition Failed")},
	{HTTP_STATUS_REQUEST_ENTITY_TOO_LONG,			("413 Request Entity Too Large")},
	{HTTP_STATUS_REQUEST_URI_TOO_LONG,				("414 Request-URI Too Long")},
	{HTTP_STATUS_UNSUPPORT_MEDIA_TYPE,				("415 Unsupported Media Type")},
	{HTTP_STATUS_REQUESTED_RANGE_NOT_SATISFIABLE,	("416 Requested Range Not Satisfiable")},
	{HTTP_STATUS_EXPECTATION_FAILED,				("417 Expectation Failed")},
	{HTTP_STATUS_UNPROCESSABLE_ENTITY,				("422 Unprocessable Entity")},
	{HTTP_STATUS_LOCKED,							("423 Locked")},
	{HTTP_STATUS_FAILED_DEPENDENCY,					("424 Failed Dependency")},
	{HTTP_STATUS_UPGRADE_REQUIRED,					("426 Upgrade Required")},
	{HTTP_STATUS_INTERNAL_SERVER_ERROR,				("500 Internal Server Error")},
	{HTTP_STATUS_NOT_IMPLEMENTED,					("501 Not Implemented")},
	{HTTP_STATUS_BAD_GATEWAY,						("502 Bad Gateway")},
	{HTTP_STATUS_SERVICE_UNAVAILABLE,				("503 Service Unavailable")},
	{HTTP_STATUS_GATEWAY_TIMEOUT,					("504 Gateway Timeout")},
	{HTTP_STATUS_VERSION_NOT_SUPPORTED,				("505 HTTP Version Not Supported")},
	{HTTP_STATUS_VARIANT_ALSO_NEGOTIATES,			("506 Variant Also Negotiates")},
	{HTTP_STATUS_INSUFFICIENT_STORAGE,				("507 Insufficient Storage")},
	{HTTP_STATUS_NOT_EXTENED,						("510 Not Extended")},
	{65535,											0},
};													
		
const char HTTP_CONTENT_TYPE_URLENCODED[] = ("application/x-www-form-urlencoded");
													
					
// HandleRequest的返回值
const int	HTTP_SUCC = 0;	// 成功
const int	HTTP_NOT_HANDLE = 1;	// 未处理
const int	HTTP_INTERNAL_ERROR = 2;	// 内部错误
const int	HTTP_ASYNC = 3;	// 异步处理

#endif //INC_CIFOX_HTTPDEFINE_H__					



