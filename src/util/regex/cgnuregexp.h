#ifndef __GNU_CREGEXP_H__
#define __GNU_CREGEXP_H__
/*
 * 这是对gnu regexp函数的c++封装
 */

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#ifdef WIN32
#include "regex.h"
#else
#include <regex.h>
#define hx_regexec regexec
#define hx_regcomp regcomp
#define hx_regfree regfree
#endif
#include <assert.h>
#include <sys/types.h>
#include <string>
#include <vector>

__NS_BASIC_START
class  CBasicRegExp : public CBasicObject
{
public:
	CBasicRegExp(const char* pattern)
	{
		_suc = (hx_regcomp(&_re, pattern, REG_EXTENDED) == 0);
		if(_suc)
		{
			_matches.resize(_re.re_nsub + 1);
		}
	}

	~CBasicRegExp()
	{
		if(_suc)
			hx_regfree(&_re);
	}

	bool  Exec(const char* src) {
		if(!_suc || src == NULL || strlen(src) <= 0)
			return false;

		_substr.erase(_substr.begin(), _substr.end());
		_src = src;
		bool bret = hx_regexec(&_re, _src.c_str(), _re.re_nsub + 1, &_matches[0], 0) == 0;
		if(bret)
		{
			for(size_t i = 0; i < _re.re_nsub + 1; i ++)
			{
				if(_matches[i].rm_eo >= _matches[i].rm_so && _matches[i].rm_so >= 0)
					_substr.push_back(_src.substr(_matches[i].rm_so, _matches[i].rm_eo - _matches[i].rm_so));
			}
		}
		return bret;
	}

	bool Exec(std::string src) {
		return Exec(src.c_str());
	}
	
	bool Exec() {
		if(!_suc || GetMatchEnd(0) <= 0)
			return false;

		std::string s = _src.substr(GetMatchEnd(0));
		return Exec(s.c_str());
	}

	const char* GetMatch(size_t index) {
		if(index >= _substr.size())
			return "";

		return _substr[index].c_str();
	}

	size_t GetMatchStart(size_t index = 0) {	
		if(index >= _substr.size())
			return 0;

		return _matches[index].rm_so;
	}

	size_t GetMatchEnd(size_t index = 0) {
		if(index >= _substr.size())
			return 0;

		return _matches[index].rm_eo;
	}

	size_t GetNumberOfMatches()
	{
		return _matches.size();
	}
protected:
	regex_t	_re;
	bool    _suc;

	std::vector<regmatch_t>  _matches;
	std::vector<std::string> _substr;	
	std::string	_src;
};

/*
 * 对字符串做模式匹配之后在替换生成一个新的字符串
 * 调用格式  gsub(原始字符串，匹配模式， 替换的字符串)
 * 如果能匹配成功，那么将用替换的字符串替换掉匹配的部分，在进行下一次匹配
 * 其中替换的字符串中可以使用%1,%2等分别代表捕获的字符串
 * 如  gsub("1234-5678", "(\d+)-(\d+)", "%2-%1") 运行的结果是 5678-1234
 * 如果不能匹配，那么返回原来的字符串
 */

inline std::string Basic_gsub(const char* src, const char* pattern, const char* rep)
{
	std::string rstr;

	const char* prep = rep;
	size_t  lastoff(0);	
	CBasicRegExp exObj(pattern);
	bool bmatch = exObj.Exec(src);
	while(bmatch)
	{
		if(exObj.GetMatchStart() > 0)
			rstr += std::string(&src[lastoff], exObj.GetMatchStart());

		prep = rep;
		while(*prep != '\0')
		{
			if(*prep == '%')
			{
				if(isdigit((unsigned char)*(prep+1)))
				{
					//assert(*(prep+1) - '0' < exObj.GetNumberOfMatches());

					rstr += exObj.GetMatch(*(prep+1) - '0');
					prep ++;
				}
				else if(*(prep+1) == '%')
				{
					rstr += '%';
					prep ++;
				}
				else
					rstr += '%';
			}
			else
				rstr += *prep;

			prep++;
		}

		lastoff += exObj.GetMatchEnd();
		
		bmatch = exObj.Exec();
	}

	if(lastoff < strlen(src))
		rstr += std::string(&src[lastoff], strlen(src) - lastoff);
	return rstr;
}
__NS_BASIC_END
#endif //__GNU_CREGEXP_H__
