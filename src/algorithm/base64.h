/***********************************************************************************************
// 文件名:     base64.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 11:19:23
// 内容描述:   base64标准编码、解码类
base64Ex 扩展base64编码、解码类（实现简单加密）
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_BASE64_H
#define BASIC_BASE64_H


__NS_BASIC_START
class CBasicBase64;		///标志Base64编码处理类
     class CBasicBaseEx;	///我们自己的特殊Base64编码处理类

//! 标准base64编码、解码处理类
class _BASIC_DLL_API CBasicBase64 : public basiclib::CBasicObject
{
public:
	CBasicBase64();
	virtual ~CBasicBase64();

	//! Base64解码函数
	/*! 
	 *\param szDecoding 需要解码的数据指针
	 *\param nSize      数据支持长度
	 *\param buf        解码数据保存,成功的时候，将解码数据追加在buf数据尾部
	 *\return           >0 成功编码之后的长度 <0 错误
	*/
	virtual long Decode(BYTE* szDecoding, int nSize, CBasicSmartBuffer& buf);
	//! Base64解码函数
	/*! 
	 *\param szDecoding 需要解码的数据指针
	 *\param nSize      数据支持长度
	 *\param pOut       解码数据输出
	 *\param nMaxSize   pOut的最大长度，返回时指明解码的数据长度，
	 *\return           >0 成功编码之后的长度 <0 错误,pOut=NULL时，返回需要的数据长度
	*/
	virtual long Decode( BYTE* szDecoding, int nSize, BYTE* pOut, int nMaxSize);

	//! Base64编码函数
	/*! 
	 *\param szDecoding 需要编码的数据指针
	 *\param nSize      数据支持长度
	 *\param buf        编码数据保存,成功的时候，将编码码数据追加在buf数据尾部
	 *\return           >0 成功编码之后的长度 <0 错误
	*/
	virtual long Encode(BYTE* szEncoding, int nSize, CBasicSmartBuffer& buf);
	//! Base64编码函数
	/*! 
	 *\param szDecoding 需要编码的数据指针
	 *\param nSize      数据支持长度
	 *\param pOut       编码数据输出
	 *\param nMaxSize   pOut的最大长度，返回时指明编码的数据长度，
	 *\return           >0 成功编码之后的长度 <0 错误,pOut=NULL时， 返回需要的数据长度
	*/
	virtual long Encode( BYTE* szEncoding, int nSize, BYTE* pOut, int nMaxSize);

protected:
	virtual BYTE GetEncodeAlphabet(int nIndex)
		{ return m_sBase64Alphabet[nIndex]; }
	virtual BYTE GetDecodeAlphabet(int nIndex)
		{ return m_sBase64Decode[nIndex]; }
	virtual char GetPadding()
		{ return base64_pad; }

protected:
	static BYTE m_sBase64Alphabet[];
	static unsigned char m_sBase64Decode[];
	static char base64_pad;	
};

//! 经过改造的base64编码、解码处理类，调用接口和CBasicBase64相同
class _BASIC_DLL_API CBasicBase64Ex : public CBasicBase64
{
public:
	CBasicBase64Ex();
	virtual ~CBasicBase64Ex();

protected:
	virtual BYTE GetEncodeAlphabet(int nIndex)
		{ return m_sBase64Alphabet[nIndex]; }
	virtual BYTE GetDecodeAlphabet(int nIndex)
		{ return m_sBase64Decode[nIndex]; }
	virtual char GetPadding()
		{ return base64_pad; }

protected:
	static BYTE m_sBase64Alphabet[];
	static unsigned char m_sBase64Decode[];
	static char base64_pad;	
};
__NS_BASIC_END

#endif 
