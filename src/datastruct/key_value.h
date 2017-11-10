/***********************************************************************************************
// 文件名:     key_valye.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 8:18:40
// 内容描述:   定义类似字符串key=value的解析和生成。用在url等请求字符串
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_KEY_VALUE_H
#define BASIC_KEY_VALUE_H
#pragma once
/////////////////////////////////////////////////////////////////////////////////////////////
//声明
/////////////////////////////////////////////////////////////////////////////////////////////
__NS_BASIC_START
#pragma warning (push)
#pragma warning (disable: 4251)

class _BASIC_DLL_API CBasicKey2Value : public basiclib::CBasicObject
{
public:
	CBasicKey2Value(int nHashSize = 7);

	bool HasValue(const char* lpszKey, CBasicString& strValue);
	CBasicString GetValue(const char* lpszKey) const;
	const char* GetValueRef(const char* lpszKey) const;

	void SetValue(const char* lpszKey, const char* lpszValue);
	void SetValue(const char* lpszKey, long lValue);

	bool RemoveKey(const char* lpszKey);

	void RemoveAll()
	{
		m_map.RemoveAll();
	}

	CBasicString& operator [](CBasicString& strKey)
	{
		strKey.MakeLower();
		return m_map[strKey];
	}

	bool IsEmpty()
	{
		return m_map.IsEmpty();
	}
public:
	//默认分隔符 =&
	void ParseTextURL(const char* pszBuffer, int nLen = -1);
	//默认分隔符 =\n
	void ParseTextLine(const char* pszBuffer, int nLen = -1);
	//外部指定分隔符
	void ParseText(const char* pszBuffer, const char* lpszTok, int nLen = -1);

	//合并成字符串
	//返回的指针为 sbBuf 里面的内容
	const char* CombineToString(CBasicSmartBuffer& sbBuf, const char* lpszTok = NULL);
public:
	template<class Function>
	void ForEach(Function  func)
	{
		CMapStringToString_S::iterator i = m_map.begin();
		for (; i != m_map.end(); i++)
		{
			func(i->first.c_str(), i->second.c_str());
		}
	}
protected:
	void _ParseText(const char* pszBuffer, const char* lpszTok, int nLen = -1);
protected:
	void SetRawValue(const char* lpszKey, const char* lpszValue);
	virtual void SetValueAt(CBasicString& strKey, const char* lpszValue);
protected:
	CMapStringToString_S	m_map;
};

#pragma warning (pop)

__NS_BASIC_END

/////////////////////////////////////////////////////////////////////////////////////////////
#endif //#define BASIC_KEY_VALUE_H
