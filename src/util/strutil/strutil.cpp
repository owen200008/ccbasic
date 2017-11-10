#include "strutil.h"
#include <stdarg.h>
#include <algorithm>
#include <locale>
#ifdef __BASICWINDOWS
#pragma comment(lib, "shlwapi.lib")
#endif

__NS_BASIC_START

char intToHexChar(int x) 
{
	static const char HEX[16] = {
			'0', '1', '2', '3',
			'4', '5', '6', '7',
			'8', '9', 'A', 'B',
			'C', 'D', 'E', 'F'};
		return HEX[x];
}

const char* Basic_LTrim(const char* str)
{
	FORWARD_SPACE(str, Null_String_S, 0);
	return str;
}
char* Basic_LTrim(char* str)
{
	FORWARD_SPACE(str, Null_String_S, 0);
	return str;
}

int Basic_ConvertStringToHexString(const char *pszSrc, int nCount, char* pszDest, int nDest)
{
	if(pszSrc == NULL || nCount <= 0 || pszDest == NULL || nDest <= 0 || nCount*2 > nDest)
	{
		return nCount << 1; // *2
	}
	int i = 0;
	for (i = 0; i < nCount; i++)
	{
		int n1 = (*(unsigned char *)(pszSrc+i)) % 16;
		int n2 = (*(unsigned char *)(pszSrc+i)) / 16;
		pszDest[2 * i] = intToHexChar(n2);
		pszDest[2 * i + 1] = intToHexChar(n1);
	}
	return nCount * 2;
}

namespace __private
{

	const char g_c_key[] = "^=,\r\n-~()[];";
	const char g_c_esc[] = "^ecrnstpPbBa";
	const char g_c_crlr[]  = "\r\n";

	const WCHAR g_w_key[] = {'^', '=', ',', '\r', '\n', '-', '~', '(', ')', '[', ']', ';', '\0'};
	const WCHAR g_w_esc[] = {'^', 'e', 'c', 'r',  'n',  's', 't', 'p', 'P', 'b', 'B', 'a', '\0'};
	const WCHAR g_w_crlr[] = {'\r', '\n', '\0'};

	const char*		__get_key_string(__private::Type2Type<char>)
	{
		return g_c_key;
	}
	const char*		__get_esc_string(__private::Type2Type<char>)
	{
		return g_c_esc;
	}

	const WCHAR*	__get_key_string(__private::Type2Type<WCHAR>)
	{
		return g_w_key;
	}
	const WCHAR*	__get_esc_string(__private::Type2Type<WCHAR>)
	{
		return g_w_esc;
	}

	const char*		__get_crlf_string(__private::Type2Type<char>)
	{
		return g_c_crlr;
	}
	const WCHAR*	__get_crlf_string(__private::Type2Type<WCHAR>)
	{
		return g_w_crlr;
	}


	struct html_specical_char_a
	{
		const char*	spec_char;
		const char*	spec_string;
		const char*	spec_key_spec;
	};
	struct html_specical_char_w
	{
		const WCHAR*	spec_char;
		const WCHAR*	spec_string;
		const WCHAR*	spec_key_spec;
	};

	const html_specical_char_a g_c_html_specialchars[] =
	{
		{"&",	"&amp;",	"amp"},
		{"\"",	"&quot;",	"quot"},
		{"'",	"&#039;",	"#039"},
		{"<",	"&lt;",		"lt"},
		{">",	"&gt;",		"gt"},
		{" ",	"&#032;",	"#032"},
		{NULL,	NULL}
	};

	const html_specical_char_w g_w_html_specialchars[] =
	{
		{(const WCHAR*)L"&",	(const WCHAR*)L"&amp;",		(const WCHAR*)L"amp"},
		{(const WCHAR*)L"\"",	(const WCHAR*)L"&quot;",	(const WCHAR*)L"quot"},
		{(const WCHAR*)L"'",	(const WCHAR*)L"&#039;",	(const WCHAR*)L"#039"},
		{(const WCHAR*)L"<",	(const WCHAR*)L"&lt;",		(const WCHAR*)L"lt"},
		{(const WCHAR*)L">",	(const WCHAR*)L"&gt;",		(const WCHAR*)L"gt"},
		{(const WCHAR*)L" ",	(const WCHAR*)L"&#032;",	(const WCHAR*)L"#032"},
		{NULL,	NULL}
	};

	const size_t HTML_SPEC_CHAR_AMPERSAND	= 0;
	const size_t HTML_SPEC_CHAR_DOUBLEQUOTE = 1;
	const size_t HTML_SPEC_CHAR_SINGLEQUOTE = 2;
	const size_t HTML_SPEC_CHAR_LESSTHAN	= 3;
	const size_t HTML_SPEC_CHAR_GREATERTHAN = 4;
	const size_t HTML_SPEC_CHAR_SPACE		= 5;

	const char*	__get_html_spec_string(char c, int quotestyle)
	{
		const char* s = NULL;
		switch(c)
		{
		case '&':
			s = g_c_html_specialchars[HTML_SPEC_CHAR_AMPERSAND].spec_string;
			break;
		case '"':
			if (quotestyle & ENT_COMPAT)
				s = g_c_html_specialchars[HTML_SPEC_CHAR_DOUBLEQUOTE].spec_string;
			break;
		case '\'':
			if (quotestyle & ENT_SQUOTE)
				s = g_c_html_specialchars[HTML_SPEC_CHAR_SINGLEQUOTE].spec_string;
			break;
		case '<':
			s = g_c_html_specialchars[HTML_SPEC_CHAR_LESSTHAN].spec_string;
			break;
		case '>':
			s = g_c_html_specialchars[HTML_SPEC_CHAR_GREATERTHAN].spec_string;
			break;
		case ' ':
			s = g_c_html_specialchars[HTML_SPEC_CHAR_SPACE].spec_string;
			break;
		}
		return s;
	}

	const WCHAR*	__get_html_spec_string(WCHAR c, int quotestyle)
	{
		const WCHAR* s = NULL;
		switch(c)
		{
		case L'&':
			s = g_w_html_specialchars[HTML_SPEC_CHAR_AMPERSAND].spec_string;
			break;
		case L'"':
			if (quotestyle & ENT_COMPAT)
				s = g_w_html_specialchars[HTML_SPEC_CHAR_DOUBLEQUOTE].spec_string;
			break;
		case L'\'':
			if (quotestyle & ENT_SQUOTE)
				s = g_w_html_specialchars[HTML_SPEC_CHAR_SINGLEQUOTE].spec_string;
			break;
		case L'<':
			s = g_w_html_specialchars[HTML_SPEC_CHAR_LESSTHAN].spec_string;
			break;
		case L'>':
			s = g_w_html_specialchars[HTML_SPEC_CHAR_GREATERTHAN].spec_string;
			break;
		case L' ':
			s = g_w_html_specialchars[HTML_SPEC_CHAR_SPACE].spec_string;
			break;
		}
		return s;
	}

	const char* __get_html_spec_char(const char* psz)
	{
		size_t i = 0;
		while(g_c_html_specialchars[i].spec_char)
		{
			if (0 == __tcscmp(psz, g_c_html_specialchars[i].spec_key_spec))
			{
				return g_c_html_specialchars[i].spec_char;
			}
			++ i;
		}
		return NULL;
	}


	const WCHAR* __get_html_spec_char(const WCHAR* psz)
	{
		size_t i = 0;
		while(g_w_html_specialchars[i].spec_char)
		{
			if (0 == __tcscmp(psz, g_w_html_specialchars[i].spec_key_spec))
			{
				return g_w_html_specialchars[i].spec_char;
			}
			++ i;
		}
		return NULL;
	}

	const char html_sep_tok_c[] = "&;";
	const WCHAR* html_sep_tok_w = (const WCHAR*)L"&;";

	const char* __get_html_sep_tok(__private::Type2Type<char>)
	{
		return html_sep_tok_c;
	}
	const WCHAR* __get_html_sep_tok(__private::Type2Type<WCHAR>)
	{
		return html_sep_tok_w;
	}
}

namespace strutil
{
	const char blank_c_string[] = " \t\r\n";
	const WCHAR* blank_w_string =  (const WCHAR*)L" \t\r\n";
	const char*	__get_blank_string(__private::Type2Type<char>)
	{
		return blank_c_string;
	}

	const WCHAR*	__get_blank_string(__private::Type2Type<WCHAR>)
	{
		return blank_w_string;
	}
}


static unsigned char put_v(unsigned char c)
{
	if (c < 10)
		return (unsigned char)'0' + c;
	else
		return (unsigned char)'A' + (c - 10);
}

char_string	Basic_URLEncode(const char* psz)
{
	char_string ret;
	char buf[3];
	memset(buf, 0, sizeof(buf));
	while(*psz)
	{
		if ((*psz >= 'a' && *psz <= 'z')
			|| (*psz >= 'A' && *psz <= 'Z')
			|| (*psz >= '0' && *psz <= '9')
			|| *psz == '-'
			|| *psz == '_')
		{
			ret += *psz;
		}
		else if (*psz == ' ')
		{
			ret += '+';
		}
		else
		{
			if (*psz)
				ret += '%';
			unsigned char c = *(unsigned char*)psz;
			buf[0] = put_v(c/16);
			buf[1] = put_v(c%16);
			ret += buf;
		}
		++ psz;
	}
	return ret;
}


long Basic_RoundString(char* lpszBuffer, long lRoundLength)
{
	long lLength = __tcslen(lpszBuffer);
	if (lLength <= lRoundLength)
		return lLength;

	long lTail = lRoundLength - 4;
	long i = lTail;
	for (; i >= 0; -- i)
	{
		if ((*(lpszBuffer + i) & 0x80) == 0)
			break;	// 不是双字节
	}
	if ((lTail - i) % 2 != 0)
	{
		-- lTail;
	}
	memcpy(lpszBuffer + lTail + 1, "...", 4);
	return lTail + 4;
}



#ifdef __BASICWINDOWS
bool Basic_StringMatchSpec(const WCHAR* pszFile, const WCHAR* pszSpec)
{
	return PathMatchSpecW(pszFile, pszSpec);
}


bool Basic_StringMatchSpec(const char* pszFile, const char* pszSpec)
{
	return PathMatchSpecA(pszFile, pszSpec);
}

int64_t __atoi64_s(const char* str, int nLen/* = -1*/)
{
	char Buff[32];
	if (nLen == -1)
	{
		nLen = __tcslen(str);
	}
	memset(Buff, 0, sizeof(Buff));

	__tcscpyn(Buff, sizeof(Buff)-1, str, nLen);
	Basic_LTrim(Buff);
#ifdef __BASICWINDOWS
	return _atoi64(Buff);
#else
	char* pEnd;
	return strtol(Buff, &pEnd, 10);
#endif
}
#endif	// __BASICWINDOWS

__NS_BASIC_END
