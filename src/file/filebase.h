/***********************************************************************************************
// 文件名:     filebase.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 12:41:35
// 内容描述:   定义基本的文件操作类
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_FILEBASE_H
#define BASIC_FILEBASE_H

#pragma once

__NS_BASIC_START
/////////////////////////////////////////////////////////////////////////////////////////////
//声明
//class CBasicObject;
	class CFileBase;
		class CDiskFile;					//磁盘文件
		class CMemFileBase;					//内存文件
/////////////////////////////////////////////////////////////////////////////////////////////
class CFileBase : public basiclib::CBasicObject
{
public:
	CFileBase() {}
	virtual ~CFileBase(){}
	//open
public:
	virtual long Open(const char* lpszFileName, DWORD nOpenFlags) = 0;
	virtual long OpenMemFile(void* pBuffer, long lCount, long lLength) = 0;
	virtual long Close() = 0;
	//status
public:
	virtual BOOL  IsOpen() const = 0;
	virtual BOOL  IsValidFile() { return true; }
	//position
public:
	virtual long GetStatus(TLFileStatus& rStatus) const = 0;
	virtual long GetPosition() const = 0;
	virtual long Seek(long lOff, DWORD nFrom) = 0;
	virtual long SetLength(long lNewLen, char cFill) = 0;
	virtual long GetLength() const = 0;
	//read write
public:
	virtual long Read(void* lpBuf, long lCount) = 0;
	virtual long Write(const void* lpBuf, long lCount, long lRepeat, char cFill) = 0;
	virtual long Flush() = 0;

	virtual CFileBase* Duplicate() const = 0;

	//only for memory files
	virtual void* GetDataBuffer() = 0;
	virtual void* GetCurDataBuffer(long& lCount) = 0;	//返回当前位置的数据

	virtual long CopyTo(const char* lpszThisFileName, const char* lpszFileName) = 0;
	virtual long CopyFrom(const char* lpszThisFileName, const char* lpszFileName) = 0;

};

/////////////////////////////////////////////////////////////////////////////
class CDiskFile : public CFileBase
{
public:
	CDiskFile();
	virtual ~CDiskFile();
	//open
public:
	virtual long Open(const char* lpszFileName, DWORD nOpenFlags);
	virtual long OpenMemFile(void* pBuffer, long lCount, long lLength);
	virtual long Close();
	//status
public:
	virtual BOOL  IsOpen() const;
	//position
public:
	virtual long GetStatus(TLFileStatus& rStatus) const;
	virtual long GetPosition() const;
	virtual long Seek(long lOff, DWORD nFrom);
	virtual long SetLength(long lNewLen, char cFill);
	virtual long GetLength() const;
	//read write
public:
	virtual long Read(void* lpBuf, long lCount);
	virtual long Write(const void* lpBuf, long lCount, long lRepeat, char cFill);
	virtual long Flush();

	virtual CFileBase* Duplicate() const;

	//only for memory files
	virtual void* GetDataBuffer();
	virtual void* GetCurDataBuffer(long& lCount);	//返回当前位置的数据

	virtual long CopyTo(const char* lpszThisFileName, const char* lpszFileName);
	virtual long CopyFrom(const char* lpszThisFileName, const char* lpszFileName);
protected:
	long WriteToFile(const void* lpBuf, long lCount);
	long _seek(long lOff, DWORD nFrom) const;
protected:
	HANDLE	m_hFile;
};

/////////////////////////////////////////////////////////////////////////////
class CMemFileBase : public CFileBase
{
public:
	CMemFileBase();
	virtual ~CMemFileBase();
	//open
public:
	virtual long Open(const char* lpszFileName, DWORD nOpenFlags);
	virtual long OpenMemFile(void* pBuffer, long lCount, long lLength);
	virtual long Close();
	//status
public:
	virtual BOOL  IsOpen() const;
	//position
public:
	virtual long GetStatus(TLFileStatus& rStatus) const;
	virtual long GetPosition() const;
	virtual long Seek(long lOff, DWORD nFrom);
	virtual long SetLength(long lNewLen, char cFill);
	virtual long GetLength() const;
	//read write
public:
	virtual long Read(void* lpBuf, long lCount);
	virtual long Write(const void* lpBuf, long lCount, long lRepeat, char cFill);
	virtual long Flush();

	virtual CFileBase* Duplicate() const;

	//only for memory files
	virtual void* GetDataBuffer();
	virtual void* GetCurDataBuffer(long& lCount);

	virtual long CopyTo(const char* lpszThisFileName, const char* lpszFileName);
	virtual long CopyFrom(const char* lpszThisFileName, const char* lpszFileName);
protected:
	char* _GetDataBuffer() const;
	long  _GetDataHead(int nIndex) const;

	void  _SetDataHead(int nIndex, long lValue);

	char* _AllocMemory(long lLength);
	void  FreeMemory();

	void ClearMember();
private:
	char*	m_pFileObj;
};

__NS_BASIC_END
//////////////////////////////////////////////////////////////////////////////////////////////////
#endif 

