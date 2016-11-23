#pragma once
#ifndef INC_CIFOX_HTTPRESPONSE_H__
#define INC_CIFOX_HTTPRESPONSE_H__

#include "../../inc/basic.h"

struct HttpStatus;
class HttpResponse : public basiclib::CBasicObject
{
public:
	typedef basiclib::basic_hash_map<basiclib::char_string, basiclib::char_string>::type	HeaderContainer;
public:
	HttpResponse(void);
	~HttpResponse(void);

	/*!
	* 设置返回的HttpStatus状态
	* \param uStatus 设置的状态值。默认200
	*/
	void	SetStatus(unsigned short uStatus);
	
	/*!
	* 获取返回的HttpStatus状态
	* \return 当前的状态
	*/
	unsigned short GetStatus() const;

	/*!
	* 设置Http版本。程序对version格式不做校验，调用者必须自行保证格式正确。
	* \param pszVersion HTTP版本信息。eg: 1.0、1.1
	*/
	void	SetVersion(const char* pszVersion);

	/*!
	* 获取版本信息
	* \return 版本字符串
	*/
	const char*	GetVersion() const;

	/*!
	* 设置Http头的值
	* \param key 键名
	* \param value 键值
	* \param replace 对于当前如果该键值已经存在的处理。true:替换; false:设置失败
	*/
	void	SetHeader(const char* key, const char* value, bool replace = true);

	/*!
	* 设置Cookie的值
	* \param key 键名
	* \param value 键值
	* \param expires 失效时间
	* \param path cookie的有效路径
	* \param domain cookie的有效域
	*/
	void	SetCookie(const char* key, const char* value, time_t expires = 0, const char* path = NULL, const char* domain = NULL);

	/*!
	* 获取已经设置的Http头的值
	* \param key 键名
	* \param def 如果找不到默认返回的值
	* \return 获取的键值
	*/
	const char* GetHeader(const char* key, const char* def = NULL) const;

	/*!
	* 追加返回包的数据
	* \param buf 指向追加的内存指针
	* \param len buf的长度
	*/
	void	AppendContent(const char* buf, size_t len);

	/*!
	* 获得所有Http头设置生成的头数据块
	* \param buffer[out]，存储返回的数据
	*/
	void	GetHeaderData(basiclib::CBasicSmartBuffer* buffer, long lContentLen = -1) const;

	/*!
	* 获得所有Http内容块的数据
	* \param buffer[out]，存储返回的数据
	*/
	void	GetContent(basiclib::CBasicSmartBuffer* buffer) const;

	/*!
	* 将返回的Http内容块压缩。同时会增加Http头Content-Encoding的类型
	*/
	bool	ZipOutput();
	
	/*!
	* 根据status生成返回的数据文本描述
	* \param status Http Status的值
	* \return 描述字符串
	*/
	static	const char*	GetStatusString(unsigned short status);

	/*!
	* 格式化时间
	* \param t 时间
	* \param gmtime 是否格式化为GMT时间
	* \return 格式化后的时间字符串
	*/
	static	basiclib::char_string	GetTimeString(time_t t, bool gmtime);

	/*!
	* 从完整的http包体解析各个内容(无Http协议第一行数据)
	* \param pBuffer 指向包体指针
	* \param len 包体长度
	* \return HTTP_SUCC成功。
	*/
	int		FromCacheBuffer(const char* pBuffer, size_t len);

	/*!
	* 生成Http包体(无Http协议第一行数据)
	* \param pBuffer[out]返回的数据包
	* \return HTTP_SUCC
	*/
	int		MakeCacheBuffer(basiclib::CBasicSmartBuffer* pBuffer) const;

	/*!
	* 判断是否KeepAlive
	* \return true: KeepAlive; false: Close
	*/
	bool	IsKeepAlive() const;
protected:
	void	__CombineHeader(basiclib::CBasicSmartBuffer* buffer, const char* key, const char* value) const;
	static	void AppendHeader(basiclib::CBasicSmartBuffer* buffer, const char* key, const char* value);

	void	FillHeader(basiclib::CBasicSmartBuffer* pBuffer, const char* key) const;

	class WrapFillHeader;
	class WrapFillResponse;
protected:
	unsigned short		m_nStatus;
	basiclib::char_string			m_strVersion;
	HeaderContainer		m_Headers;
	basiclib::CBasicSmartBuffer		m_bufContent;
	bool				m_bZipped;
};


#endif // INC_CIFOX_HTTPRESPONSE_H__



