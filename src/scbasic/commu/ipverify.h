#ifndef INC_IPVERIFY_H_
#define INC_IPVERIFY_H_

#include "../../inc/basic.h"
#include "../scbasic_head.h"

#pragma warning (push)
#pragma warning (disable: 4251)
#pragma warning (disable: 4275)
/////////////////////////////////////////////////////////////////////////////////////////////
// 1、支持IP列表及子网认证
// 2、调用接口时传入的IP内部自动认证是否为有效IP地址
// 3、规则串定义：
// 如：*;192.168.0.1:255.255.255.0@网关;10.0.1.51:255.255.255.0@本机
//		*代表允许全部 名称无用，只为方便阅读
/////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_IP_ITEM			(int)4				// 

struct _SCBASIC_DLL_API _IPRuler
{
	BYTE		m_szIP[MAX_IP_ITEM];			// IP地址
	BYTE		m_szMask[MAX_IP_ITEM];			// 子网掩码
	BYTE		m_szResult[MAX_IP_ITEM];		// IP与子网掩码与(&)操作结果
	
	_IPRuler()
	{
		memset(this, 0, sizeof(_IPRuler));
	}
};

//支持IP地址认证
class _SCBASIC_DLL_API CIpVerify : public basiclib::CBasicObject
{
public:
	CIpVerify();
	virtual ~CIpVerify();
	
public:
	virtual int SetIPRuler(const char* lpszRuler);
	virtual BOOL IsIpTrust(const char* lpszIP);
	
protected:
	BOOL AddIPRuler(const char* lpszIP, const char* lpszMask);
	BOOL IsIPAddr(const char* lpszIP, BYTE* szIP, int cbIP);
	void EmptyRuler();

protected:
	basiclib::CPtrList				m_lsIpRuler;			// IP规则列表
	BOOL							m_bSupportAll;			// 允许全部 (*)
	basiclib::CCriticalSection		m_synObj;				// 同步对象
};

/************************************************************************************************
支持域名和端口的认证匹配
规则：地址1:端口1;地址2:端口2
      172.20.0.153:8601;172.20.0.*:8601;*:8601;www.abcd.com:1234
*************************************************************************************************/
//字符串匹配规则
class _SCBASIC_DLL_API CBasicStringCmpInfo : public basiclib::CBasicObject
{
public:
	CBasicStringCmpInfo();
	virtual ~CBasicStringCmpInfo();

	//设置规则
	virtual long InitRuleInfo(const char* lpszRule, const char* lpszSplit = ";");

	//加入规则
	BOOL AddRuleInfo(basiclib::CBasicString& strRuleInfo);

	//判断是否在范围内
	long IsInRule(const char* lpszData, long lIndex = 0);

	//获取状态
	virtual void GetStatus(basiclib::CBasicString& strInfo);
protected:
	typedef basiclib::basic_vector<basiclib::CBasicStringArray> ContainRuleInfo;
	typedef ContainRuleInfo::iterator							ContainRuleInfoIterator;
	ContainRuleInfo  m_ayRuleInfo;

	basiclib::CBasicString m_strInfo;
};

//基于字符串匹配规则的,整型匹配
class _SCBASIC_DLL_API CIpDomainVerify : public CBasicStringCmpInfo
{
public:
	CIpDomainVerify();
	virtual ~CIpDomainVerify();

	//设置规则
	virtual long InitRuleInfo(const char* lpszRule, const char* lpszSplit = ";");

	//获取状态
	virtual void GetStatus(basiclib::CBasicString& strInfo);

	//是否是信任IP和端口
	BOOL IsTrust(const char* lpszData, WORD wPort);

	//
	basiclib::CBasicString GetPortInfo(int nIndex);
protected:
	typedef basiclib::basic_vector<unsigned short>				ContainPort;
	typedef basiclib::basic_vector<ContainPort>					ContainNumber;
	typedef ContainNumber::iterator               ContainNumberIterator;

	ContainNumber                                m_ayRuleExtre;
	BOOL                                         m_bTrustAll;  //信任所有
};
#pragma warning (pop)
#endif



