/***********************************************************************************************
// 文件名:     inifile.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012-2-22 23:41:54
// 内容描述:   
处理ini文件的类,重写数据存放模式，
使用新的解析函数ParseIni提高解析速度

数据支持：
1.ini文件最大支持4GB
2.key最大支持1024字节
3.值最大支持4MB字节
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_INIFILE_H
#define BASIC_INIFILE_H 

#include <map>
#include <string>
#include <algorithm>
#include <stdio.h>

__NS_BASIC_START
//ini文件解析器
#define INI_TYPE_SCETION		0x00000001	//section 开始
#define INI_TYPE_KEY			0x00000002	//值行
#define INI_TYPE_SCETION_END	0x00000004	//section 结束
#define INI_COMMENT				0x00000008	//注释行

template<typename F>
long ParseIniChar(const char* pszData, long cbData, F f, long cbBeginPos = 0)
{
	long nLines = 0;
	long nType = 0;
	long nBeginPos = 0;
	long nLineBegin = cbBeginPos;
	long nNameEnd = 0;
	long nSecBegin = 0;
	long nSpaceNum = 0;
	CBasicString strLastSection;
	char d = '\0';
	int i = 0;
	for (i = cbBeginPos; i < cbData; ++i)
	{
		d = *(pszData + i);

		if (d == '[')
		{
			if (nType == 0 && (nSpaceNum >= i - nLineBegin))
			{
				nType = INI_TYPE_SCETION;
				nBeginPos = i + 1;
				if (!strLastSection.IsEmpty())
				{
					if (!f(INI_TYPE_SCETION_END, strLastSection.c_str(), strLastSection.c_str(), 0, strLastSection.GetLength(), nSecBegin, nLineBegin))
						break;
				}
				nSecBegin = i; //section开始位置
			}
		}
		else if (d == ']')
		{
			if (nType == INI_TYPE_SCETION)
			{
				strLastSection = CBasicString(pszData + nBeginPos, i - nBeginPos);
				if (!f(INI_TYPE_SCETION, strLastSection.c_str(), pszData, nBeginPos, i, -1, -1))
					break;
				nType = 0;
			}
		}
		else if (d == '\r' || d == '\n')
		{
			if (nType == INI_TYPE_KEY)
			{
				if (!f(INI_TYPE_KEY, strLastSection.c_str(), pszData, nLineBegin, nNameEnd, nNameEnd + 1, i))
					break;
			}

			if (d == _T('\n'))
			{
				++nLines;
				nLineBegin = i + 1;	//新行开始位置
			}

			nType = 0;
			nSpaceNum = 0;
		}
		else if (nType == 0 && d == '=')
		{
			nType = INI_TYPE_KEY;
			nNameEnd = i;
		}
		else if (nType == 0 && (d == ';' || d == '#'))	//判断注释行
		{
			nType = INI_COMMENT;
		}
		else if (nType == 0 && (d == ' ' || d == '\t'))
		{
			++nSpaceNum;
		}
	}

	if (!strLastSection.IsEmpty())
	{
		f(INI_TYPE_SCETION_END, strLastSection.c_str(), strLastSection.c_str(), 0, strLastSection.GetLength(), nSecBegin, i);
	}
	return nLines;
}



#ifdef _WIN32
#define		NEW_LINE			"\r\n"
#define		NEW_LINE_LEN		2 * sizeof(char)
#else	
#define		NEW_LINE			"\n"
#define		NEW_LINE_LEN		1 * sizeof(char)
#endif

#define		SEP_SIGN			"||"

//64字节的数据存放偏移量
struct IndexData
{
	union
	{
		LONGLONG  m_nData64;		//64位数据
		struct  
		{
			unsigned int m_nOffset;	//数据偏移量
			union
			{				
				unsigned int m_nSectionLength;	//段长度
				struct  
				{
					//针对特殊需要，修改这两个值可以改变各自支持的数据长度，
					//注意，两者相加等于32
					unsigned int m_nKeyLen : 10;	//值名称长度,最长1024字节
					unsigned int m_nValueLen : 22;	//值长度，最长2^22=4MB字节，
				};
			};
		};
	};
};

namespace __private
{
	template<typename F>
	class get_section_strChar
	{
	public:
		get_section_strChar(F f) :m_f(f){};
		bool operator()(long nType, const char* lpszSection, const char* lpszData, long nV1Begin, long nV1End, long nV2Begin, long nV2End)
		{
			if (nType == INI_TYPE_KEY)
			{
				CBasicString strName(lpszData + nV1Begin, nV1End - nV1Begin);
				CBasicString strValue(lpszData + nV2Begin, nV2End - nV2Begin);
				strName.TrimLeft();
				strName.TrimRight();

				m_f(strName, strValue);
			}
			return true;
		}
	protected:
		F m_f;
	};

	//遍历所有的段
	template<typename F>
	class get_section_list_strChar
	{
	public:
		get_section_list_strChar(F f) :m_f(f){};
		bool operator()(long nType, const char* lpszSection, const char* lpszData, long nV1Begin, long nV1End, long nV2Begin, long nV2End)
		{
			if (nType == INI_TYPE_SCETION)
			{
				m_f(lpszSection);
			}
			return true;
		}
	protected:
		F m_f;
	};
}

#pragma warning (push)
#pragma warning (disable: 4251)
class _BASIC_DLL_API CBasicIniOp
{
public:
	CBasicIniOp()
	{
		m_bModified = false;
	}
	CBasicIniOp(const char* lpszFileName)
	{
		m_bModified = false;
		InitFromFile(lpszFileName);
	}
	virtual ~CBasicIniOp()
	{
		Empty();
	};
    //!get size
    int GetIniSize();
public:
	typedef		 basic_map<CBasicString, LONGLONG>::type   Dict;		//单独可以的对应关系	
	Dict				m_dic;					///通过key查找数据的数据结构		
	CBasicString		m_file;				///默认的ini文件名，保留最后读取的文件名

	bool				m_bModified;			///判断数据是否被修改过
	CBasicSmartBuffer	m_buffer;			//ini文件的缓存
	CBasicSmartBuffer	m_tmp;				///临时数据
protected:
	char* GetDataBuffer()
	{
		return (char*)m_buffer.GetDataBuffer();
	}
	long   GetDataLength()
	{
		return m_buffer.GetDataLength() / sizeof(char);
	}
public:
	///清空当前数据
	void Empty();

	///判断当前是否有读取过数据
	///数据为空返回true，否则返回false
	bool IsEmpty()
	{
		return m_buffer.GetDataLength() <= 0;
	}

	///返回数据是否被修改过
	bool IsModified()
	{
		return m_bModified;
	}

	/// 从数据文件初始化数据字典
	/// >= 0读取成功
	/// -1 读取文件失败
	/// -2 文件不存在
	int InitFromFile(const char* filename);
	/// 从内存数据中初始化【ANSI 版本】
	int InitFromMem(const char* lpszData, size_t cbData);

	///遍历一个section，f作为回调，带两个参数，原型如void f(LPCTSTR key，LPCTSTR value)
	///每搜索到一个key，就会调用f一次，不是有效的key=value行不回调，如注释等
	template<typename F>
	int  GetSection(const char* lpszSection, F f)
	{
		ASSERT(lpszSection);
		CBasicString strSection = lpszSection;
		strSection.MakeUpper();
		Dict::iterator iter = m_dic.find(strSection);
		if (iter == m_dic.end())
		{
			return -1;
		}

		IndexData i;
		i.m_nData64 = iter->second;
		char* pszData = GetDataBuffer();
		__private::get_section_strChar<F> _callback(f);
		return ParseIniChar(pszData + i.m_nOffset, i.m_nSectionLength, _callback);
	}
	///删除一个section
	//如果section为NULL，那么删除整个配置文件
	bool EmptySection(const char* lpszSection);

	//遍历所有的段名，f作为回调，带一个参数，原型为 void f(LPCTSTR lpszSection);
	template<typename F>
	int GetSectionList(F f)
	{
		__private::get_section_list_strChar<F> _callback(f);
		return ParseIniChar(GetDataBuffer(), GetDataLength(), _callback);
	}

	///插入值到[key1]findkey2之前
	///如果findkey2不存在，那么加入段最后
	///插入值时，原来的[key1]key2值将被删除
	///key1, key2,value, findkey2均不能为null
	///成功返回0， 失败返回-1
	int InsertBefore(const char* key1, const char* key2, const char* value, const char* findkey2);

	//设置值，如果value=NULL,那么删除该值
	void SetData(const char* key1, const char* key2, const char* value)
	{
		_SetData(key1, key2, value);
	}

	void SetLong(const char* key1, const char* key2, long lValue)
	{
		char szBuf[128];
		sprintf(szBuf, "%d", (Net_Int)lValue);
		_SetData(key1, key2, szBuf);
	}

	void SetDouble(const char* key1, const char* key2, double fValue)
	{
		char szBuf[128];
		sprintf(szBuf, "%f", fValue);
		_SetData(key1, key2, szBuf);
	}

	CBasicString GetData(const char* key1, const char* key2, const char* defval = "")
	{
		CBasicString strRet = GetSelfData(key1, key2, defval);
		strRet.TrimLeft();
		strRet.TrimRight();
		return strRet;
	}

	///取数据
	double GetDouble(const char* key1, const char* key2, const char* defval = "")
	{
		return atof(GetData(key1, key2, defval).c_str());
	}

	///取long型数据
	long  GetLong(const char* key1, const char* key2, const char* defval = "")
	{
		return atoi(GetData(key1, key2, defval).c_str());
	}

	LONG64 __atoi64Char(const char* str, int nLen/* = -1*/)
	{
		char Buff[32];
		if (nLen == -1)
		{
			nLen = strlen(str);
		}
		memset(Buff, 0, sizeof(Buff));
		strncpy_s(Buff, sizeof(Buff)-1, str, nLen);
		Basic_LTrim(Buff);
#ifdef __BASICWINDOWS
		return _atoi64(Buff);
#else
		char* pEnd;
		return strtol(Buff, &pEnd, 10);
#endif
	}
	__int64 GetInt64(const char* key1, const char* key2, __int64 nDefault = 0)
	{
		CBasicString strRet = GetData(key1, key2);
		if (strRet.GetLength() <= 0)
		{
			return nDefault;
		}
		return __atoi64Char(strRet.c_str(), strRet.GetLength());
	}
	
	///
	virtual CBasicString GetSelfData(const CBasicString& section, const CBasicString& key, const char* defval = "")
	{
		CBasicString strKey = section + SEP_SIGN + key;
		strKey.MakeUpper();
		Dict::iterator iter = m_dic.find(strKey);
		if (iter == m_dic.end())
		{
			return defval;
		}

		IndexData i;
		i.m_nData64 = iter->second;
		char* pszData = GetDataBuffer();
		return CBasicString(pszData + i.m_nOffset + i.m_nKeyLen + 1, i.m_nValueLen);
	}

	//! 把内容写入文件
	bool WriteToFile(const char* filename = NULL);



	/*!将当前内容合并到文件filename中，将用当前的数据合并文件中的数据
	*\param  需要合并的文件
	*\return 返回处理的数据行数
	*\remark 当前ini文件中的注释信息会被丢掉
	*/
	long CombinToFile(const char* filename)
	{
		CBasicIniOp ini(filename);

		long lCount = ParseIniChar(GetDataBuffer(), GetDataLength(), [&](long nType, const char* lpszSection, const char* lpszData, long nV1Begin, long nV1End, long nV2Begin, long nV2End)->bool{
			return ini.ParseCombine(nType, lpszSection, lpszData, nV1Begin, nV1End, nV2Begin, nV2End);
		});

		ini.WriteToFile(filename);
		return lCount;
	}

	/*!将当前对象中的数据合并到其他对象中，优先级是当前数据 > ini
	*\param ini 需要合并的CWBasicIniOp对象
	*\return 返回处理的数据行数
	*\remark 当前对象中的注释信息将被忽略
	*/
	long combine_to_other(CBasicIniOp& ini)
	{
		return ParseIniChar(GetDataBuffer(), GetDataLength(), [&](long nType, const char* lpszSection, const char* lpszData, long nV1Begin, long nV1End, long nV2Begin, long nV2End)->bool{
			return ini.ParseCombine(nType, lpszSection, lpszData, nV1Begin, nV1End, nV2Begin, nV2End);
		});
	}

	/*!两个对象之间的拷贝
	*/
	CBasicIniOp& operator =(const CBasicIniOp& ini)
	{
		m_buffer = ini.m_buffer;
		m_dic = ini.m_dic;
		m_file = ini.m_file;
		return *this;
	}
protected:
	//合并方式的ini解析
	bool ParseCombine(long nType, const char* lpszSection, const char* lpszData, long nV1Begin, long nV1End, long nV2Begin, long nV2End);

	/*!解析ini文件的回调函数*/
	bool ParseINI(long nType, const char* lpszSection, const char* lpszData, long nV1Begin, long nV1End, long nV2Begin, long nV2End);

	/*!内部赋值操作*/
	void _SetData(const char* key1, const char* key2, const char* value);

	/*!将需要变动的数据的偏移量改变*/
	void UpdateOffset(DWORD dwBeginOffset, long nMove);

	/// 从内存数据中初始化
	/// 内部函数调用，不提供对外支持
	int InitData(const char* lpszData, size_t cbData);

	/*增加数据
	\param dwStartPos 起点位置
	\param cbOld      原始数据长度
	\param lpszNewData 新数据
	\param cbData      新数据长度俄
	*/
	void AddNewData(DWORD dwStartPos, long cbOldData, const char* lpszNewData, long cbData);

	/// 从内存数据中初始化【ANSI 版本】
	int _InitData(const char* lpszData, size_t cbData);

	//! 根据回调函数，写入数据
	void _WriteTo(const std::function<long(const char*, long)>& func);

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma warning (pop)


#ifndef __BASICWINDOWS
BOOL  WritePrivateProfileString(LPCTSTR lpszAppName, LPCTSTR lpszKeyName, LPCTSTR lpszString, LPCTSTR lpszFilename);
DWORD GetPrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName,LPCTSTR lpDefault,LPTSTR lpReturnedString, DWORD nSize, LPCTSTR lpFileName);
int   GetPrivateProfileInt( LPCSTR lpAppName, LPCSTR lpKeyName, INT nDefault, LPCSTR lpFileName);
#endif

__NS_BASIC_END
#endif
