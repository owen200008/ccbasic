#pragma once
#ifndef BASIC_basicallocator_H__
#define BASIC_basicallocator_H__

#include "../inc/basic_def.h"
#include "mem.h"
#include <memory>

__NS_BASIC_START

//! basicallocator，使用BasicAllocate和TL_Deallcate的内存分配器

// TEMPLATE CLASS allocator
template<class _Ty>
class basicallocator
{	// generic allocator for objects of class _Ty
public:
	typedef size_t				size_type;
	typedef ptrdiff_t			difference_type;
	typedef _Ty					value_type;
	typedef value_type*			pointer;
	typedef value_type&			reference;
	typedef const value_type*	const_pointer;
	typedef const value_type&	const_reference;

	template<class _Other>
	struct rebind
	{	// convert an allocator<_Ty> to an allocator <_Other>
		typedef basicallocator<_Other> other;
	};

	pointer address(reference _Val) const
	{	// return address of mutable _Val
		return (&_Val);
	}

	const_pointer address(const_reference _Val) const
	{	// return address of nonmutable _Val
		return (&_Val);
	}

	basicallocator() 
	{	// construct default allocator (do nothing)
	}

	basicallocator(const basicallocator<_Ty>&) 
	{	// construct by copying (do nothing)
	}

	template<class _Other>
	basicallocator(const basicallocator<_Other>&) 
	{	// construct from a related allocator (do nothing)
	}

	template<class _Other>
	basicallocator<_Ty>& operator=(const basicallocator<_Other>&)
	{	// assign from a related allocator (do nothing)
		return (*this);
	}

	void deallocate(pointer _Ptr, size_type)
	{	// deallocate object at _Ptr, ignore size
		BasicDeallocate(_Ptr);
	}

	pointer allocate(size_type _Count)
	{	// allocate array of _Count elements
		return (pointer)(BasicAllocate(sizeof(_Ty)*_Count));
	}

	pointer allocate(size_type _Count, const void *)
	{	// allocate array of _Count elements, ignore hint
		return (allocate(_Count));
	}

	void construct(pointer *_Ptr)
	{	// default construct object at _Ptr
		::new ((void *)_Ptr) _Ty();
	}
	void construct(pointer _Ptr, const _Ty& _Val)
	{	// construct object at _Ptr with value _Val
		//_Construct(_Ptr, _Val);
		::new(_Ptr)_Ty(_Val);
	}
	template<class _Objty,
	class... _Types>
		void construct(_Objty *_Ptr, _Types&&... _Args)
	{	// construct _Objty(_Types...) at _Ptr
		::new (_Ptr) _Objty(std::forward<_Types>(_Args)...);
	}

	void destroy(pointer _Ptr)
	{	// destroy object at _Ptr
		_Ptr->~_Ty();
	}

	size_type max_size() const
	{	// estimate maximum array size
		size_type _Count = (size_t)(-1) / sizeof (_Ty);
		return (0 < _Count ? _Count : 1);
	}
};


// allocator TEMPLATE OPERATORS
template<class _Ty,
class _Other> inline
	bool operator==(const basicallocator<_Ty>&, const basicallocator<_Other>&)
{	// test for allocator equality (always true)
	return (true);
}

template<class _Ty,
class _Other> inline
	bool operator!=(const basicallocator<_Ty>&, const basicallocator<_Other>&)
{	// test for allocator inequality (always false)
	return (false);
}


// CLASS allocator<void>
template<> class basicallocator<void>
{	// generic allocator for type void
public:
	typedef void _Ty;
	typedef _Ty *pointer;
	typedef const _Ty *const_pointer;
	typedef _Ty value_type;

	template<class _Other>
	struct rebind
	{	// convert an allocator<void> to an allocator <_Other>
		typedef basicallocator<_Other> other;
	};

	basicallocator()
	{	// construct default allocator (do nothing)
	}

	basicallocator(const basicallocator<_Ty>&)
	{	// construct by copying (do nothing)
	}

	template<class _Other>
	basicallocator(const basicallocator<_Other>&)
	{	// construct from related allocator (do nothing)
	}

	template<class _Other>
	basicallocator<_Ty>& operator=(const basicallocator<_Other>&)
	{	// assign from a related allocator (do nothing)
		return (*this);
	}
};



//! 创建一个对象，并调用构造函数
/*
\date	2008-05-30
\return 类型为T的对象指针
*/
template<class T>
T*	Basic_NewObject()
{
	void* p = BasicAllocate(sizeof(T));
	::new (p)T;
	return (T*)p;
}

//! 释放一个对象，并调用析构函数
/*
\date	2008-05-30
\param	obj	需要释放的对象指针
*/
template<class T>
void BASIC_DeleteObject(T* obj)
{
	obj->~T();
	BasicDeallocate(obj);
}

#if _MSC_VER >= 1200
#define DECLARE_TL_DELETE_EX\
	void operator delete(void* p, void* pPlace);

#define IMPLEMENT_TL_DELETE_EX(class_name)\
	void class_name::operator delete(void* p, void*)\
	{\
		BasicDeallocate(p);\
	}
#else
#define DECLARE_TL_DELETE_EX
#define IMPLEMENT_TL_DELETE_EX(a)
#endif

#define DECLARE_TL_MEM\
	public:\
	void* operator new(size_t nSize);\
	void operator delete(void* p);\
	DECLARE_TL_DELETE_EX

#define IMPLEMENT_TL_MEM(class_name)\
	void* class_name::operator new(size_t nSize)\
	{\
		return BasicAllocate(nSize);\
	}\
	\
        void class_name::operator delete(void* p)\
	{\
		BasicDeallocate(p);\
	}\
	IMPLEMENT_TL_DELETE_EX(class_name)

__NS_BASIC_END

#endif
