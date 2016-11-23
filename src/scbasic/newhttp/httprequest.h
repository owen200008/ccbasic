#pragma once
#ifndef INC_CIFOX_HTTPREQUEST_H__
#define INC_CIFOX_HTTPREQUEST_H__

#include "httpparser.h"

// 上下文的处理类 add by mojiayong
class CContextHandle : public basiclib::CBasicObject
{
public:
	// param[in] pContent 需要处理内容
	CContextHandle(void* pContent) : m_pContent(pContent){}
	//! param[in] pContext 处理过程中用到的上下文
	//! param[in][out] buffer 处理后放置的内容 
	virtual long HandleContext(void* pContext, basiclib::CBasicSmartBuffer& buffer) = 0;
protected:
	void* m_pContent;
};


class HttpRequest : public IHttpParseHandler
{
public:
	typedef basiclib::basic_hash_map<basiclib::char_string, basiclib::char_string>::type	HeaderContainer;
	typedef basiclib::basic_hash_map<basiclib::char_string, CContextHandle*>::type			ContextHandleContainer;
public:
	HttpRequest(void);
	~HttpRequest(void);

public:
	// 获取请求方式.
	// GET,POST等
	virtual void	MethodHandler(const char* method);
	// 获取URI
	// /index.php?a=b&c=d
	virtual void	URIHandler(const char* URI);
	// 获取协议
	// HTTP/1.1
	virtual void	ProtocolHandler(const char* protocol);
	// 得到http头
	// Accept-Encoding: gzip, deflate
	virtual void	HeaderHandler(const char* key, const char* value);
	// 获得post的内容
	virtual void	PostDataHandler(const char* data, size_t len);
	// 一个请求包解析结束
	virtual void	RequestEndHandler();
	// 获得原始请求数据[not implement]
	virtual const char* GetRequestData(const char *szkey) const;

public:
	// 取得header头的值
	bool	GetHeaderValue(const char* key, basiclib::char_string& value) const;
	const char*	GetHeaderValue(const char* key) const;
    bool	GetHeaderValue(const char* key, basiclib::CTime& value) const;
	bool	GetHeaderValue(const char* key, long& value) const;

	// 取得Cookie的值
	const char* GetCookie(const char* key) const;

	// 获得协议头的信息
	const basiclib::char_string&	GetMethod() const;
	const basiclib::char_string&	GetURI() const;
	const basiclib::char_string&	GetProtocol() const;

	const basiclib::char_string&	GetFileName() const;

	// 获得GET方式的头信息
	const char* GetParamValue(const char* key, const char* pDefault = NULL) const;
	// 
	const char* GetContentData() const;
	unsigned long GetContentLength() const;

	bool	CanCache() const;

	template<class Functor>
	void	ForEachParam(Functor func) const
	{
		for_each(m_Params.begin(), m_Params.end(), func);
	}

	template<class Functor>
	void	ForEachHeader(Functor func) const
	{
		for_each(m_Headers.begin(), m_Headers.end(), func);
	}

	// 设置上下文的一个传递信息
	void	SetContext(void* pContext);
	void*	GetContext() const;

	// KeepAlive
	BOOL	IsKeepAlive() const;
	BOOL	IsSupportGZip() const;

	// 重置
	void	Reset();

	// 设置对端地址
	void		SetPeerAddr(const char* addr);
	const char*	GetPeerAddr() const;
	basiclib::CBasicString& GetRefPeerAddr();

	void	SetFileName(const char* file);

public: // 上下文的处理
	//! 增加上下文处理
	void	AddContextHandle(const char* szKey, CContextHandle* pHandle);
	//! 根据关键字删除上下文处理
	void	RemoveContextHandle(const char* szKey);
	//! 调用处理函数，返回一个buffer
	long	HandleContext(const char* szKey, void* pContext, basiclib::CBasicSmartBuffer& buffer) const;

//	static	void	DecodeString(const char* strIn, char_string& strOut);
protected:
	void	ParseURI(const char* URI);
	
	CContextHandle* FindHandle(const char* szKey) const;
protected:
	basiclib::char_string		m_strMethod;
	basiclib::char_string		m_strURI;
	basiclib::char_string		m_strProtocol;

	basiclib::char_string		m_strFile;
	basiclib::char_string		m_strArgs;

	basiclib::CBasicString		m_strPeerAddr;

	HeaderContainer	m_Params;
	HeaderContainer	m_Cookies;
	HeaderContainer	m_Headers;
	basiclib::CBasicStaticBuffer	m_bufBody;

	void*			m_pContext;
	ContextHandleContainer m_conHandle;
};

#endif // INC_CIFOX_HTTPREQUEST_H__



