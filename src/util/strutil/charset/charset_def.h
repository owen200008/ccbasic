/***********************************************************************************************
// 文件名:     charset_def.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 11:20:30
// 内容描述:   
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_CHARSET_DEF_H
#define BASIC_CHARSET_DEF_H

#pragma once

#include "../../../inc/basic_def.h"

__NS_BASIC_START

//#ifdef BASIC_CP_ACP
//#undef BASIC_CP_ACP
//#undef BASIC_CP_OEMCP
//#undef BASIC_CP_MACCP
//#undef BASIC_CP_THREAD_ACP
//#undef BASIC_CP_SYMBOL
//#undef BASIC_CP_UTF7
//#undef BASIC_CP_UTF8
//#endif

const unsigned int BASIC_CP_ACP				= 0;		// default to ANSI code page
const unsigned int BASIC_CP_OEMCP			= 1;           // default to OEM  code page
const unsigned int BASIC_CP_MACCP			= 2;           // default to MAC  code page
const unsigned int BASIC_CP_THREAD_ACP		= 3;           // current thread's ANSI code page
const unsigned int BASIC_CP_SYMBOL			= 42;          // SYMBOL translations

const unsigned int CP_037 			= 37;
const unsigned int CP_SJIS			= 932;		// Japanese Shift-JIS
const unsigned int CP_GBK 			= 936;		// Simplified Chinese GBK
const unsigned int CP_MS949 		= 949;		// Korean
const unsigned int CP_BIG5			= 950;		// Traditional Chinese BIG5
const unsigned int CP_UNICODE		= 1200;	
const unsigned int CP_UTF16 		= 1200;
const unsigned int CP_UTF16_BE		= 1201;		

const unsigned int BASIC_CP_UTF7			= 65000;   // UTF-7 translation
const unsigned int BASIC_CP_UTF8			= 65001;

const unsigned int CP_EUC_JP 		= 51932;
const unsigned int CP_EUC_KR		= 51949;
const unsigned int CP_GB18030 		= 54936;
const unsigned int CP_GB2312 		= 52936;
const unsigned int CP_US_ACSII 		= 20127;
const unsigned int CP_US_ASCII2		= 65000;

// SBCS(Single Byte Character Set) CodePages
const unsigned int CP_WINDOWS_1250 	= 1250;	// Central Europe
const unsigned int CP_WINDOWS_1251	= 1251;	// Cyrillic
const unsigned int CP_WINDOWS_1252	= 1252;	// Latin I
const unsigned int CP_WINDOWS_1253	= 1253;	// Greek
const unsigned int CP_WINDOWS_1254	= 1254;	// Turkish
const unsigned int CP_WINDOWS_1255	= 1255;	// Hebrew
const unsigned int CP_WINDOWS_1256	= 1256;	// Arabic
const unsigned int CP_WINDOWS_1257 	= 1257;	// Baltic
const unsigned int CP_WINDOWS_1258 	= 1258;	// Vietnam
const unsigned int CP_WINDOWS_THAI	= 874;		// Thai

// DBCS(Double Byte Character Set) Codepages
const unsigned int CP_WINDOWS_SJIS	= 932;		// Japanese Shift-JIS
const unsigned int CP_WINDOWS_GBK	= 936;		// Simplified Chinese GBK
const unsigned int CP_WINDOWS_MS949	= 949;		// Korean
const unsigned int CP_WINDOWS_BIG5	= 950;		// Traditional Chinese Big5


// OEM Code pages
const unsigned int CP_OEM_US		= 437;		// US
const unsigned int CP_OEM_ARABIC	= 720;		// Arabic
const unsigned int CP_OEM_GREEK		= 737;		// Greek
const unsigned int CP_OEM_BALTIC	= 775;		// Baltic
const unsigned int CP_OEM_LATINI	= 850;		// Multilingual Latin I
const unsigned int CP_OEM_LATINII	= 852;		// Latin II
const unsigned int CP_OEM_CYRILLIC	= 855;		// Cyrillic
const unsigned int CP_OEM_TURKISH	= 857;		// Turkish
const unsigned int CP_OEM_LATINI_EU	= 858;		// Multilingual Latin I + Euro
const unsigned int CP_OEM_HEBREW	= 862;		// Hebrew
const unsigned int CP_OEM_RUSSIAN	= 866;		// Russian

// the following codepage are used as both windows ANSI and OEM codepages
const unsigned int CP_OEM_THAI		= 874;		// Thai
const unsigned int CP_OEM_SJIS		= 932;		// Japanese Shift-JIS
const unsigned int CP_OEM_GBK		= 936;		// Simplified Chinese GBK
const unsigned int CP_OEM_MS949		= 949;		// Korean
const unsigned int CP_OEM_BIG5		= 950;		// Traditional Chinese Big5
const unsigned int CP_OEM_VIETNAM	= 1258;	// Vietnam


const unsigned int CP_ISO_2022_JP1	= 50220;
const unsigned int CP_ISO_2022_JP2	= 50221;
const unsigned int CP_ISO_2022_JP3	= 50222;
const unsigned int CP_ISO_2022_KR 	= 50225;
const unsigned int CP_ISO_8859_1 	= 28591;
const unsigned int CP_ISO_8859_2 	= 28592;
const unsigned int CP_ISO_8859_3	= 28593;
const unsigned int CP_ISO_8859_4	= 28594;
const unsigned int CP_ISO_8859_5 	= 28595;
const unsigned int CP_ISO_8859_6 	= 28596;
const unsigned int CP_ISO_8859_7 	= 28597;
const unsigned int CP_ISO_8859_8 	= 28598;
const unsigned int CP_ISO_8859_9 	= 28599;
const unsigned int CP_JOHAB 		= 1361;
const unsigned int CP_KOI8_R 		= 20866;
const unsigned int CP_MAC_ARABIC	= 10004;
const unsigned int CP_MAC_CENTRAL_EUROPE	= 10029;
const unsigned int CP_MAC_CHINESE_SIMPLE	= 10008;
const unsigned int CP_MAC_CHINESE_TRADITIONAL = 10002;
const unsigned int CP_MAC_CROATIAN 	= 10082;
const unsigned int CP_MAC_CYRILLIC 	= 10007;
const unsigned int CP_MAC_GREEK 	= 10006;
const unsigned int CP_MAC_HEBREW	= 10005;
const unsigned int CP_MAC_ICELAND	= 10079;
const unsigned int CP_MAC_JAPAN		= 10001;
const unsigned int CP_MAC_KOREAN	= 10003;
const unsigned int CP_MAC_ROMAN		= 10000;
const unsigned int CP_MAC_ROMANIA 	= 10010;
const unsigned int CP_MAC_THAI 		= 10021;
const unsigned int CP_MAC_TURKISH	= 10081;
const unsigned int CP_MAC_UKRAINE	= 10017;


const char CPS_ACP[]		= "gbk";
const char CPS_UTF16LE[]	= "utf-16le";
const char CPS_SJIS[]		= "shift_jis";	// Japanese Shift-JIS
const char CPS_GBK[]		= "gbk";
const char CPS_BIG5[]		= "big5";
const char CPS_UTF16[]		= "utf-16";
const char CPS_UTF16BE[]	= "utf-16be";
const char CPS_UTF7[]		= "utf-7";
const char CPS_UTF8[]		= "utf-8";
const char CPS_EUC_JP[]		= "euc_jp";
const char CPS_EUC_KR[]		= "euc_kr";
const char CPS_GB18030[]	= "gb18030";
const char CPS_GB2312[]		= "gb2312";
const char CPS_ASCII[]		= "ascii";
const char CPS_UTF32[]		= "utf-32";
const char CPS_UTF32LE[]	= "utf-32le";
const char CPS_UTF32BE[]	= "utf-32be";

const char CPS_ISO_2022_JP1[]	= "iso-2002-jp";
const char CPS_ISO_2022_JP2[]	= "iso-2002-jp1";
const char CPS_ISO_2022_JP3[]	= "iso-2002-jp2";
const char CPS_ISO_2022_KR[]	= "iso-2022-kr";
const char CPS_ISO_8859_1[]		= "iso-8859-1";
const char CPS_ISO_8859_2[]		= "iso-8859-2";
const char CPS_ISO_8859_3[]		= "iso-8859-3";
const char CPS_ISO_8859_4[]		= "iso-8859-4";
const char CPS_ISO_8859_5[]		= "iso-8859-5";
const char CPS_ISO_8859_6[]		= "iso-8859-6";
const char CPS_ISO_8859_7[]		= "iso-8859-7";
const char CPS_ISO_8859_8[]		= "iso-8859-8";
const char CPS_ISO_8859_9[]		= "iso-8859-9";
const char CPS_JOHAB[]			= "johab";
const char CPS_KOI8_R[]			= "koi8_r";
const char CPS_MAC_ARABIC[]		= "MacArabic";
const char CPS_MAC_CENTRAL_EUROPE[] = "MacCentralEurope";
const char CPS_MAC_CHINESE_SIMPLE[] = "MacChineseSimple";
const char CPS_MAC_CHINESE_TRADITIONAL[] = "MacChineseTraditional";
const char CPS_MAC_CROATIAN[]	= "MacCroatian";
const char CPS_MAC_CYRILLIC[]	= "MacCyrillic";
const char CPS_MAC_GREEK[]		= "MacGreek";
const char CPS_MAC_HEBREW[]		= "MacHebrew";
const char CPS_MAC_ICELAND[]	= "MacIceland";
const char CPS_MAC_JAPAN[]		= "MacJapan";
const char CPS_MAC_KOREAN[]		= "MacKorean";
const char CPS_MAC_ROMAN[]		= "MacRoman";
const char CPS_MAC_ROMANIA[]	= "MacRomania";
const char CPS_MAC_THAI[]		= "MacThai";
const char CPS_MAC_TURKISH[]	= "MacTurkish";
const char CPS_MAC_UKRAINE[]	= "MacUkraine";

const char ICONV_TARGET_APPEND[]	= "//TRANSLIT//IGNORE";

__NS_BASIC_END

#endif 

