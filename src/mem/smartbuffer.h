/***********************************************************************************************
// 文件名:     smartbuffer.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 11:23:31
// 内容描述:   内存块管理
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_SMARTBUFFER_H
#define BASIC_SMARTBUFFER_H

#pragma	pack(1)
__NS_BASIC_START
//////////////////////////////////////////////////////////////////////////////////////////////
//class CBasicObject;
//	class CWBasicSmartBuffer;						//可变长度的内存块
//////////////////////////////////////////////////////////////////////////////////////////////
//! 可变长度的内存块
/*!
*  
*/
class _BASIC_DLL_API CBasicSmartBuffer : public CBasicObject
{
public:
	CBasicSmartBuffer();
	CBasicSmartBuffer(const basiclib::CBasicSmartBuffer &smBuf);
	virtual ~CBasicSmartBuffer();
public:
	//binddatatosmartbuffer,绑定的数据外部就不能使用了，里面的数据会被改变
	void BindOutData(char* pData, int nLength);

	BOOL IsEmpty() const { return m_pszBuffer == NULL; }
	void Free();

	char* GetDataBuffer(long& lLength) const;
	char* GetDataBuffer() const { return m_pszBuffer; }

	long GetDataLength() const{ return m_cbBuffer; }
	void SetDataLength(long lLength);

	char* AppendData(const char* pszData, long lLength);
	char* AppendDataEx(const char* pszData, long lLength);
	char* CommitData(long lLength);		//增加数据的长度（必要时重新分配内存）。返回原来的指针，用于填充数据

	void AppendString(const char* lpszText);
	void AppendData(long lVal);
	void AppendData(double fVal);

	BOOL InitFormFile(const char* lpszFile);

	CBasicSmartBuffer& operator = (const CBasicSmartBuffer& buffer);
	friend bool operator == (CBasicSmartBuffer& b1, CBasicSmartBuffer& b2);

	CBasicSmartBuffer& operator << (const char* lpszText)
	{
		AppendString(lpszText);
		return *this;
	}
	//读取数据
	void ReadData(void* pData, int nLength);

	//设置和获取长度
	long GetAllocBufferLength(){ return m_cbAlloc; }
protected:
	char* AllocBuffer(long lLength, long lGrowLength = 0);
	void  EmptyBuffer();
protected:
	bool	m_bSelfBuf;
	char*	m_pszBuffer;
	long	m_cbAlloc;
	long	m_cbBuffer;
};
bool operator == (CBasicSmartBuffer& b1, CBasicSmartBuffer& b2);
////////////////////////////////////////////////////////////////////////////////////////////////////////
__NS_BASIC_END
//////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack()
#endif 
