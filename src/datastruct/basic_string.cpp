#include "../inc/basic.h"
#include <assert.h>
#include <algorithm>
#include <stdarg.h>
#ifdef _WIN32
#include <malloc.h>
#endif
#include "../util/strutil/strutil.h"
#include "../util/strutil/charset.h"

#if defined(__BASICWINDOWS) && !defined(__SGI_SBASIC_PORT)
#include <xhash>
#endif

#if defined(__LINUX)  || defined(__ANDROID)
#include <stdarg.h>
#endif

#ifdef _WIN32
#pragma warning(disable: 4996)
#pragma warning(disable: 4267)
#endif

__NS_BASIC_START
CBasicString::CBasicString()
{

}

CBasicString::CBasicString(char ch, int nRepeat)
: tstring_s(nRepeat, ch)
{
}

CBasicString::CBasicString(const char* lpsz)
{
	if (NULL != lpsz)
	{
		assign(lpsz);
	}
}

CBasicString::CBasicString(const char* lpch, int nLength)
{
	if (NULL != lpch && nLength > 0)
	{
		assign(lpch, nLength);
	}
}

// get data length
int CBasicString::GetLength() const
{
	return length();
}
// TRUE if zero length
BOOL CBasicString::IsEmpty() const
{
	return empty();
}

// clear contents to empty
void CBasicString::Empty()
{
	clear();
}

// return single character at zero-based index
char CBasicString::GetAt(int nIndex) const
{
	return at(nIndex);
}

// set a single character at zero-based index
void CBasicString::SetAt(int nIndex, char ch)
{
	assert(nIndex >= 0 && nIndex < GetLength());
	*(begin() + nIndex) = ch;
}

// straight character comparison
int CBasicString::Compare(const char* lpsz) const
{
	return strcmp(c_str(), lpsz);
}

// compare ignoring case
int CBasicString::CompareNoCase(const char* lpsz) const
{
	return strcmp(c_str(), lpsz);
}


// find character starting at left, -1 if not found
int CBasicString::Find(char ch) const
{
	return find(ch);
}
// find character starting at right
int CBasicString::ReverseFind(char ch) const
{
	return rfind(ch);
}
// find character starting at zero-based index and going right
int CBasicString::Find(char ch, int nStart) const
{
	return find(ch, nStart);
}
// find first instance of any character in passed string
int CBasicString::FindOneOf(const char* lpszCharSet) const
{
	return find_first_of(lpszCharSet);
}
// find first instance of substring
int CBasicString::Find(const char* lpszSub) const
{
	return find(lpszSub);
}
// find first instance of substring starting at zero-based index
int CBasicString::Find(const char* lpszSub, int nStart) const
{
	return find(lpszSub, nStart);
}

// remove whitespace starting from right edge
void CBasicString::TrimRight()
{
	strutil::rtrim(*this);
}

// remove whitespace starting from left side
void CBasicString::TrimLeft()
{
	strutil::ltrim(*this);
}


// NLS aware conversion to uppercase
void CBasicString::MakeUpper()
{
	strutil::makeupper(*this);
}

// NLS aware conversion to lowercase
void CBasicString::MakeLower()
{
	strutil::makelower(*this);
}

// reverse string right-to-left
void CBasicString::MakeReverse()
{
	reverse(begin(), end());
}

// release buffer, setting length to nNewLength (or to first nul if -1)
void CBasicString::ReleaseBuffer(int nNewLength)
{
	if (nNewLength == -1)
		nNewLength = __tcslen(c_str());
	resize(nNewLength);
}

// get pointer to modifiable buffer exaCBasicy as long as nNewLength
char* CBasicString::GetBufferSetLength(int nNewLength)
{
	resize(nNewLength);
	return (char*)c_str();
}
// get pointer to modifiable buffer at least as long as nMinBufLength
char* CBasicString::GetBuffer(int nMinBufLength)
{
	if ((size_t)nMinBufLength > length())
		resize(nMinBufLength);
	return (char*)c_str();
}


void CBasicString::Format(const char* lpszFormat, ...)
{
	va_list argList;
	va_start(argList, lpszFormat);
	FormatV(lpszFormat, argList);
	va_end(argList);
}

#ifdef _WIN32
#define TCHAR_ARG   TCHAR
#define WCHAR_ARG   WCHAR
#define CHAR_ARG    char
#else
#define TCHAR_ARG	int
#define WCHAR_ARG	int
#define CHAR_ARG	int
#endif

#ifdef _X86_
struct _AFX_DOUBLE  { BYTE doubleBits[sizeof(double)]; };
struct _AFX_FLOAT   { BYTE floatBits[sizeof(float)]; };
#define DOUBLE_ARG  _AFX_DOUBLE
#else
#define DOUBLE_ARG  double
#endif

#define FORCE_ANSI      0x10000
#define FORCE_UNICODE   0x20000
#define FORCE_INT64     0x40000
#ifdef __BASICWINDOWS
#include <mbstring.h>
#endif
void CBasicString::FormatV(const char* lpszFormat, va_list argList)
{
#if defined(__MAC) || defined(__LINUX)
	va_list argListSave;
	va_copy(argListSave, argList);
#else
	va_list argListSave = argList;
#endif

	// make a guess at the maximum length of the resulting string
	int nMaxLen = 0;
	bool bExchange64 = false;
	for (const char* lpsz = lpszFormat; *lpsz != '\0'; lpsz = (char*)_tcsinc_s((const unsigned char*)lpsz))
	{
		// handle '%' character, but watch out for '%%'
		if (*lpsz != '%' || *(lpsz = (char*)_tcsinc_s((const unsigned char*)lpsz)) == '%')
		{
			nMaxLen += __tcslen(lpsz);
			continue;
		}

		int nItemLen = 0;

		// handle '%' character with format
		int nWidth = 0;
		for (; *lpsz != '\0'; lpsz = (char*)_tcsinc_s((const unsigned char*)lpsz))
		{
			// check for valid flags
			if (*lpsz == '#')
				nMaxLen += 2;   // for '0x'
			else if (*lpsz == '*')
				nWidth = va_arg(argList, int);
			else if (*lpsz == '-' || *lpsz == '+' || *lpsz == '0' ||
				*lpsz == ' ')
				;
			else // hit non-flag character
				break;
		}
		// get width and skip it
		if (nWidth == 0)
		{
			// width indicated by
			nWidth = atoi(lpsz);
			for (; *lpsz != '\0' && isdigit(*lpsz); lpsz = (char*)_tcsinc_s((const unsigned char*)lpsz))
				;
		}
		assert(nWidth >= 0);

		int nPrecision = 0;
		if (*lpsz == '.')
		{
			// skip past '.' separator (width.precision)
			lpsz = (char*)_tcsinc_s((const unsigned char*)lpsz);

			// get precision and skip it
			if (*lpsz == '*')
			{
				nPrecision = va_arg(argList, int);
				lpsz = (char*)_tcsinc_s((const unsigned char*)lpsz);
			}
			else
			{
				nPrecision = atoi(lpsz);
				for (; *lpsz != '\0' && isdigit(*lpsz); lpsz = (char*)_tcsinc_s((const unsigned char*)lpsz))
					;
			}
			assert(nPrecision >= 0);
		}

		// should be on type modifier or specifier
		int nModifier = 0;
		if (strncmp(lpsz, "I64", 3) == 0)
		{
			lpsz += 3;
			nModifier = FORCE_INT64;
#if (defined(__LINUX) || defined(__ANDROID))
			bExchange64 = true;
#endif
		}

		if (strncmp(lpsz, "lld", 3) == 0)
		{
			lpsz += 2;
			nModifier = FORCE_INT64;
#ifdef __BASICWINDOWS
			bExchange64 = true;
#endif
		}
		else
		{
			switch (*lpsz)
			{
				// modifiers that affect size
			case 'h':
				nModifier = FORCE_ANSI;
				lpsz = (char*)_tcsinc_s((const unsigned char*)lpsz);
				break;
			case 'l':
				nModifier = FORCE_UNICODE;
				lpsz = (char*)_tcsinc_s((const unsigned char*)lpsz);
				break;

				// modifiers that do not affect size
			case 'F':
			case 'N':
			case 'L':
				lpsz = (char*)_tcsinc_s((const unsigned char*)lpsz);
				break;
			}
		}

		// now should be on specifier
		switch (*lpsz | nModifier)
		{
			// single characters
		case 'c':
		case 'C':
			nItemLen = 2;
			va_arg(argList, CHAR_ARG);
			break;
		case 'c' | FORCE_ANSI:
		case 'C' | FORCE_ANSI:
			nItemLen = 2;
			va_arg(argList, CHAR_ARG);
			break;
		case 'c' | FORCE_UNICODE:
		case 'C' | FORCE_UNICODE:
			nItemLen = 2;
			va_arg(argList, WCHAR_ARG);
			break;

			// strings
		case 's':
		{
					const char* pstrNextArg = va_arg(argList, const char*);
					if (pstrNextArg == NULL)
						nItemLen = 6;  // "(null)"
					else
					{
						nItemLen = __tcslen(pstrNextArg);
						nItemLen = max(1, nItemLen);
					}
		}
			break;

		case 'S':
		{

					LPWSTR pstrNextArg = va_arg(argList, LPWSTR);
					if (pstrNextArg == NULL)
						nItemLen = 6;  // "(null)"
					else
					{
						nItemLen = __tcslen(pstrNextArg);
						nItemLen = max(1, nItemLen);
					}
		}
			break;

		case 's' | FORCE_ANSI:
		case 'S' | FORCE_ANSI:
		{
								 const char* pstrNextArg = va_arg(argList, const char*);
								 if (pstrNextArg == NULL)
									 nItemLen = 6; // "(null)"
								 else
								 {
									 nItemLen = __tcslen(pstrNextArg);
									 nItemLen = max(1, nItemLen);
								 }
		}
			break;

		case 's' | FORCE_UNICODE:
		case 'S' | FORCE_UNICODE:
		{
									LPWSTR pstrNextArg = va_arg(argList, LPWSTR);
									if (pstrNextArg == NULL)
										nItemLen = 6; // "(null)"
									else
									{
										nItemLen = __tcslen(pstrNextArg);
										nItemLen = max(1, nItemLen);
									}
		}
			break;
		}

		// adjust nItemLen for strings
		if (nItemLen != 0)
		{
			if (nPrecision != 0)
				nItemLen = min(nItemLen, nPrecision);
			nItemLen = max(nItemLen, nWidth);
		}
		else
		{
			switch (*lpsz)
			{
				// integers
			case 'd':
			case 'i':
			case 'u':
			case 'x':
			case 'X':
			case 'o':
				if (nModifier & FORCE_INT64)
#ifdef _WIN32
					va_arg(argList, __int64);
#else
					va_arg(argList, long long);
#endif
				else
					va_arg(argList, int);
				nItemLen = 32;
				nItemLen = max(nItemLen, nWidth + nPrecision);
				break;

			case 'e':
			case 'g':
			case 'G':
				va_arg(argList, DOUBLE_ARG);
				nItemLen = 128;
				nItemLen = max(nItemLen, nWidth + nPrecision);
				break;

			case 'f':
			{
						double f;
						char* pszTemp;

						// 312 == __tcslen("-1+(309 zeroes).")
						// 309 zeroes == max precision of a double
						// 6 == adjustment in case precision is not specified,
						//   which means that the precision defaults to 6
#ifdef _WIN32
						pszTemp = (char*)_alloca(max(nWidth, 312 + nPrecision + 6));
#else
						pszTemp = (char*)BasicAllocate(max(nWidth, 312 + nPrecision + 6));
#endif

						f = va_arg(argList, double);
						sprintf(pszTemp, "%*.*f", nWidth, nPrecision + 6, f);
						nItemLen = __tcslen(pszTemp);
#ifndef _WIN32
						BasicDeallocate(pszTemp);
#endif
			}
				break;

			case 'p':
				va_arg(argList, void*);
				nItemLen = 32;
				nItemLen = max(nItemLen, nWidth + nPrecision);
				break;

				// no output
			case 'n':
				va_arg(argList, int*);
				break;

			default:
				assert(FALSE);  // unknown formatting option
			}
		}

		// adjust nMaxLen for output nItemLen
		nMaxLen += nItemLen;
	}

	tstring_s strFormat;
	if (bExchange64)
	{
		strFormat = lpszFormat;
#ifdef __BASICWINDOWS
		strutil::replace(strFormat, "%lld", "%I64d");
#else
		strutil::replace(strFormat, "%I64d", "%lld");
#endif
		lpszFormat = strFormat.c_str();
	}

	char* pStr = GetBuffer(nMaxLen);
	if (vsprintf(pStr, lpszFormat, argListSave) > (int)length())
	{
		assert(false);
	}
	ReleaseBuffer();
	va_end(argListSave);
}

// return nCount characters starting at zero-based nFirst
CBasicString CBasicString::Mid(int nFirst, int nCount) const
{
	return substr(nFirst, nCount).c_str();
}

// return all characters starting at zero-based nFirst
CBasicString CBasicString::Mid(int nFirst) const
{
	return substr(nFirst).c_str();
}

// return first nCount characters in string
CBasicString CBasicString::Left(int nCount) const
{
	return substr(0, nCount).c_str();
}

// return nCount characters from end of string
CBasicString CBasicString::Right(int nCount) const
{
	return substr(length() - nCount, nCount).c_str();
}


// replace occurrences of chOld with chNew
int CBasicString::Replace(char chOld, char chNew)
{
	return strutil::replace(*this, chOld, chNew);
}

// replace occurrences of substring lpszOld with lpszNew;
// empty lpszNew removes instances of lpszOld
int CBasicString::Replace(const char* lpszOld, const char* lpszNew)
{
	return strutil::replace(*this, lpszOld, lpszNew);
}

// remove continuous occurrences of chTarget starting from right
void CBasicString::TrimRight(char chTarget)
{
	strutil::rtrim(*this, chTarget);
}
// remove continuous occcurrences of characters in passed string,
// starting from right
void CBasicString::TrimRight(const char* lpszTargets)
{
	strutil::rtrim(*this, lpszTargets);
}

// remove continuous occurrences of chTarget starting from left
void CBasicString::TrimLeft(char chTarget)
{
	strutil::ltrim(*this, chTarget);
}
// remove continuous occcurrences of characters in
// passed string, starting from left
void CBasicString::TrimLeft(const char* lpszTargets)
{
	strutil::ltrim(*this, lpszTargets);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

__NS_BASIC_END

