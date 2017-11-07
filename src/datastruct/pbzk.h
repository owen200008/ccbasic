#ifndef BASIC_PBZK_H
#define BASIC_PBZK_H

#define PBZK_END_STRING 0xFFFF

#pragma warning (push)
#pragma warning (disable: 4251)
#pragma warning (disable: 4275)
class _BASIC_DLL_API CPBZK
{
public:
	CPBZK();
	virtual ~CPBZK();

	void ReadPBZKFileBuffer(const char* pBuffer, int nLength);
	void AddPBZKToMap(basiclib::CBasicStringArray& ayItems);
	//判断是否存在敏感词
	bool IsContainPBZK(const char* txt, int nLength, bool bDeep = false, bool bCheckSpecialZF = false);
	//发现直接替换
	void ReplacePBZK(char* txt, int nLength, char cReplace = '*', bool bDeep = true, bool bCheckSpecialZF = false);
protected:
	//! 判断是否有非法字符
	int CheckPBZKExist(const char* txt, int nLength, int nBeginIndex, bool bDeep = false, bool bCheckSpecialZF = false);
	
protected:
	typedef basiclib::basic_map<uint16_t, void*>											HashMapPBZK;
	typedef HashMapPBZK::iterator															HashMapPBZKIterator;
	HashMapPBZK m_mapPBZK;
};
#pragma warning (pop)

#endif