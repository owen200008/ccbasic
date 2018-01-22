/***********************************************************************************************
// 文件名:     sha1.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2013-11-18 10:46:58
// 内容描述:   
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_SHA1_H
#define BASIC_SHA1_H

__NS_BASIC_START

_BASIC_DLL_API void SHA1_Perform(BYTE *indata, DWORD inlen, BYTE *outdata);
_BASIC_DLL_API void SHA1_Perform2(BYTE *indata, DWORD inlen, BYTE *indata2, DWORD inlen2, BYTE *outdata);

_BASIC_DLL_API bool Basic_AES10_encrypt(const char* pIn, char* pOut, long lDatalen, const char* pKey);
_BASIC_DLL_API bool Basic_AES10_decrypt(const char* pIn, char* pOut, long lDatalen, const char* pKey);

__NS_BASIC_END

#endif 
