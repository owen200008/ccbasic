/***********************************************************************************************
// 文件名:     rc5.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012-2-22 23:55:10
// 内容描述:   定义 RC5 的加解密算法函数
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_RC5_H
#define BASIC_RC5_H

#pragma once
#pragma	pack(1)
__NS_BASIC_START
/////////////////////////////////////////////////////////////////////////////////////////////
//声明

//! 使用RC5算法加解密数据
/*!
*\param pOutData 输出数据的缓冲区xml
*\param pInData  输入数据的缓冲区，可以和 pOutData 是相同的
*\param lDatalen 数据的长度。必须是8的整数倍。
*\param pKey     加密的密钥。
*\param iKeylen  密钥的长度。可以是 8 16
*\param iEncrypt = 1 加密  = 0 解密
*\param nRounds  加密的轮数。可以选择 8 12 16 。 轮数越多，加密强度越大。加密速度就越慢。
*\return 成功: 返回 0  不成功返回  < 0
*\remarks 本加密算法强度比DES低，但是速度是DES的3倍左右。
*\warning 如果数据长度不是8的整数倍，返回错误。
*/

class IBasicSecurity{
public:
    virtual void Check() = 0;
};

_BASIC_DLL_API int Basic_RC5_ecb_encrypt(basiclib::IBasicSecurity* pSecurity, char *pOutData, char *pInData, long lDatalen, const char *pKey, int iKeylen, int iEncrypt, int nRounds = 8);

/////////////////////////////////////////////////////////////////////////////////////////////
__NS_BASIC_END
//////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack()
#endif 
