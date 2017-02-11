#ifndef BASIC_PBZK_H
#define BASIC_PBZK_H

#define PBZK_END_STRING "IsEnd"
class CPBZK
{
public:
	CPBZK();
	virtual ~CPBZK();

	void ReadPBZKFileBuffer(const char* pBuffer, int nLength, bool bAddZiFu = true);
	void AddPBZKToMap(basiclib::CBasicStringArray& ayItems);
	//判断是否存在敏感词
	bool IsContainPBZK(const char* txt, int nLength, bool bDeep = false);
	//发现直接替换
	void ReplacePBZK(char* txt, int nLength, char cReplace = '*', bool bDeep = true);
protected:
	//! 判断是否有非法字符
	int CheckPBZKExist(const char* txt, int nLength, int nBeginIndex, bool bDeep = false);
	
protected:
	typedef basiclib::basic_map<basiclib::CBasicString, void*>								HashMapPBZK;
	typedef HashMapPBZK::iterator															HashMapPBZKIterator;
	HashMapPBZK m_mapPBZK;
};

#endif