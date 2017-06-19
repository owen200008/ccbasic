/***********************************************************************************************
// 文件名:     tlrefptr.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012-2-21 23:34:17
// 内容描述:   具有垃圾回收功能的的智能指针
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_TLREFPTR_H
#define BASIC_TLREFPTR_H

#include "../inc/basic.h"

__NS_BASIC_START

template<class T>
class CBasicRefPtr
{
public:
	typedef T	value_type;

	CBasicRefPtr() : m_pT(NULL)
	{
	}
	CBasicRefPtr(T* pT) : m_pT(pT)
	{
		if (m_pT)
			m_pT->AddRef();
	}

	template<class U>
	operator CBasicRefPtr<U>() const
	{
		//STATIC_CHECK(SUPERSUBCLASS(U, T), U_must_be_super_class_of_T);
		return CBasicRefPtr<U>(dynamic_cast<U*>(m_pT));
	}

	CBasicRefPtr(const CBasicRefPtr& ptr) : m_pT(ptr.m_pT)
	{
		if (m_pT)
			m_pT->AddRef();
	}


	~CBasicRefPtr()
	{
		if (m_pT)
			m_pT->DelRef();
	}

	CBasicRefPtr&	operator=(T* p)
	{
		if (m_pT)
		{
			m_pT->DelRef();
		}

		m_pT = p;
		if (m_pT)
		{
			m_pT->AddRef();
		}
		return *this;
	}

	CBasicRefPtr&	operator=(const CBasicRefPtr& ptr)
	{
		if (m_pT)
		{
			m_pT->DelRef();
		}

		m_pT = ptr.m_pT;
		if(m_pT)
		{
			m_pT->AddRef();
		}
		return *this;
	}

	bool		operator==(const CBasicRefPtr& ptr) const
	{
		return m_pT == ptr.m_pT;
	}

	bool		operator!=(const CBasicRefPtr& ptr) const
	{
		return m_pT != ptr.m_pT;
	}


	bool		operator==(const void* p) const
	{
		return m_pT == p;
	}

	bool		operator!=(const void* p) const
	{
		return m_pT != p;
	}
	
	T* operator->() const
	{
		return m_pT;
	}
	T* GetResFunc()
	{
		return m_pT;
	}
#ifdef __BASICWINDOWS
private:
#else
public:
#endif
	CBasicRefPtr<T>*	operator & ()
	{
		return this;
	}
#ifdef __BASICWINDOWS
	friend  basiclib::basic_list<CBasicRefPtr<T> >;
	friend  std::list<CBasicRefPtr<T> >;
#else
#endif

private:
	T*		m_pT;
};

template<class T>
class EnableRefPtr
{
public:
	EnableRefPtr()
	{
		m_lRef = 0;
	}
	virtual ~EnableRefPtr()
	{}

	void AddRef()
	{
		BasicInterlockedIncrement(&m_lRef);
	}

	bool DelRef()
	{
		if (0 == BasicInterlockedDecrement(&m_lRef)){
			delete this;
			return true;
		}
		return false;
	}
	void KnowDelRef() {
		BasicInterlockedDecrement(&m_lRef);
	}

	CBasicRefPtr<T>	GetRefPtr()
	{
		return CBasicRefPtr<T>(dynamic_cast<T*>(this));
	}

	long	GetRef() const
	{
		return m_lRef;
	}
private:
	long	m_lRef;
};

template<class T>
class CBasicRefPtrWrapper : public EnableRefPtr<CBasicRefPtrWrapper<T> >
{
public:
	CBasicRefPtrWrapper(T* p) : m_pT(p){}
	
	virtual ~CBasicRefPtrWrapper()
	{
		if (m_pT)
			delete m_pT;
	}
	T* operator->() const
	{
		return m_pT;
	}

	T* GetT() const
	{
		return m_pT;
	}

	operator T*() const
	{
		return m_pT;
	}
protected:
	T*	m_pT;
};

__NS_BASIC_END
#endif 
