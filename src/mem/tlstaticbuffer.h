#pragma once
#ifndef BASIC_TLSTATICBUFFER_H__
#define BASIC_TLSTATICBUFFER_H__

__NS_BASIC_START

struct _BASIC_DLL_API BasicStaticBuffer
{
public:
	void*		Alloc(unsigned short len);
	BOOL		IsEmpty() const;
	BOOL		Assign(unsigned short len, unsigned char c);
	void		Release();
	void*		AppendBuffer(const void* buf, unsigned short len);
	void*		AppendString(const char* str, unsigned short len);
	void		Clone(const BasicStaticBuffer& buffer);

	void*		GetBuffer() const;
	size_t		GetLength() const;
	const char*	GetString() const;

	void*		SetLength(unsigned short len);
protected:
	void*		AllocBuffer(unsigned short len);
protected:
	void*		m_pBuffer;
	unsigned short m_nLength;
	unsigned short m_nAlloc;
};

class _BASIC_DLL_API CBasicStaticBuffer
{
public:
	CBasicStaticBuffer();
	~CBasicStaticBuffer(void);

	BOOL	FromFile(const char* lpszFile);
	void	Release();

	BOOL	Assign(size_t len, unsigned char c);
	BOOL	IsEmpty() const;
	void*	SetLength(size_t len);
	void*	Alloc(size_t len);
	void	Clone(const CBasicStaticBuffer& buffer);

	void*	AppendBuffer(const void* buf, size_t len);
	void*	AppendString(const char* str, size_t len);

	void*		GetBuffer() const;
	size_t		GetLength() const;
	const char*		GetString() const;

	void	Attach(void* buffer, size_t length);
	void	Detach();

protected:
	void*	m_pBuffer;
	size_t	m_lLength;
};
__NS_BASIC_END

#endif
