/***********************************************************************************************
// 文件名:     list.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 9:15:46
// 内容描述:   实现list的MFC like接口。仅用于兼容，不建议使用。
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_LIST_H
#define BASIC_LIST_H

#pragma once

#include "../inc/basic_def.h"
#include <list>
#include <algorithm>
#include <assert.h>
#include "../types/types.h"
using namespace std;

__NS_BASIC_START
//! list类
/*! 基于std::list实现MFC like的接口。<br/>
	\attention 仅用于兼容。不建议在新项目中使用。<br/>*/
template<class TYPE, class ARG_TYPE>
class CList : public list<TYPE, DEFAULT_ALLOCATOR<TYPE> >
{
public:
	typedef typename list<TYPE, DEFAULT_ALLOCATOR<TYPE> >::iterator iterator;
	typedef typename list<TYPE, DEFAULT_ALLOCATOR<TYPE> >::const_iterator const_iterator;
	typedef CList<TYPE, ARG_TYPE> _Self;
// Construction
	CList();

// Attributes (head and tail)
	/*! count of elements*/
	int GetCount() const; //! count of elements in list</br>
	/*! if list is emtpy*/
	BOOL IsEmpty() const;

	// peek at head or tail
	//! Returns the head element of the list (cannot be empty).
	TYPE& GetHead();
	//! Returns the head element of the list (cannot be empty).
	TYPE GetHead() const;

	//! Returns the tail element of the list (cannot be empty).
	TYPE& GetTail();
	//! Returns the tail element of the list (cannot be empty).
	TYPE GetTail() const;

// Operations
	//! get head or tail (and remove it) - don't call on empty list !
	TYPE RemoveHead();
	//! get head or tail (and remove it) - don't call on empty list !
	TYPE RemoveTail();

	//! add before head 
	POSITION AddHead(ARG_TYPE newElement);
	//! add after tail
	POSITION AddTail(ARG_TYPE newElement);

	//! add another list of elements before head
	void AddHead(CList* pNewList);
	//! add another list of elements after tail
	void AddTail(CList* pNewList);

	//! remove all elements
	void RemoveAll();

	// iteration
	//! 返回头部用于POSITION
	POSITION GetHeadPosition() const;
	//! 返回尾部用于POSITION
	POSITION GetTailPosition() const;

	//! return *Position++
	TYPE& GetNext(POSITION& rPosition); 
	
	//! return *Position++
	TYPE GetNext(POSITION& rPosition) const; 

	//! return *Position--
	TYPE& GetPrev(POSITION& rPosition); 
	
	//! return *Position--
	TYPE GetPrev(POSITION& rPosition) const; 

	// getting/modifying an element at a given position
	//! getting an element at a given position
	TYPE& GetAt(POSITION position);
	//! getting an element at a given position
	TYPE GetAt(POSITION position) const;
	//! Modify an element at a given position
	void SetAt(POSITION pos, ARG_TYPE newElement);
	//! Remove an element at a given position
	void RemoveAt(POSITION position);

	// inserting before or after a given position
	//! inserting before a given position
	POSITION InsertBefore(POSITION position, ARG_TYPE newElement);
	//! inserting after a given position
	POSITION InsertAfter(POSITION position, ARG_TYPE newElement);

	//! helper functions (note: O(n) speed)
	//! defaults to starting at the HEAD, return NULL if not found
	POSITION Find(ARG_TYPE searchValue, POSITION startAfter = NULL);
	

	//! get the 'nIndex'th element (may return NULL)
	POSITION FindIndex(int nIndex) const;
	

protected:
	POSITION GetPosition(const_iterator iter) const
	{
#if defined(__GNUC__) || defined(__SGI_SBASIC_PORT)	// SGI 实现
		return (POSITION)iter._M_node;
#else
		return (POSITION)iter._Mynode();
#endif
	}
 
	POSITION GetPosition(iterator iter) const
	{
#if defined(__GNUC__) || defined(__SGI_SBASIC_PORT)	// SGI 实现
		return (POSITION)iter._M_node;
#else
		return (POSITION)iter._Mynode();
#endif
	}

	iterator GetIterator(POSITION position) const
	{
#if defined(__GNUC__) || defined(__SGI_SBASIC_PORT)	// SGI 实现
		typedef typename iterator::_Node* _Nodeptr;
		return iterator((_Nodeptr)position);
#else // P.J. 版本实现
		return iterator((_Nodeptr)position, this);
#endif
	}

	const_iterator GetConstIterator(POSITION position) const
	{
#if defined(__GNUC__) || defined(__SGI_SBASIC_PORT)	// SGI 实现
		typedef typename const_iterator::_Node* _Nodeptr;
		return iterator((_Nodeptr)position);
#else
		return const_iterator((_Nodeptr)position, this);
#endif
	}

public:
	~CList();

};


/////////////////////////////////////////////////////////////////////////////
// CList<TYPE, ARG_TYPE> inline functions

template<class TYPE, class ARG_TYPE>
inline int CList<TYPE, ARG_TYPE>::GetCount() const
{
	return _Self::size();
}

template<class TYPE, class ARG_TYPE>
inline BOOL CList<TYPE, ARG_TYPE>::IsEmpty() const
{
	return _Self::empty();
}

template<class TYPE, class ARG_TYPE>
inline TYPE& CList<TYPE, ARG_TYPE>::GetHead()
{
	return _Self::front();
}

template<class TYPE, class ARG_TYPE>
inline TYPE CList<TYPE, ARG_TYPE>::GetHead() const
{
	assert(!_Self::empty());
	return _Self::front();
}

template<class TYPE, class ARG_TYPE>
inline TYPE& CList<TYPE, ARG_TYPE>::GetTail()
{
	assert(!_Self::empty());
	return _Self::back();
}

template<class TYPE, class ARG_TYPE>
inline TYPE CList<TYPE, ARG_TYPE>::GetTail() const
{
	assert(!_Self::empty());
	return _Self::back();
}

template<class TYPE, class ARG_TYPE>
inline TYPE& CList<TYPE, ARG_TYPE>::GetNext(POSITION& rPosition) // return *Position++
{
	iterator iter = GetIterator(rPosition);
	TYPE& v = *iter;
	++ iter;
	if (iter == _Self::end())
		rPosition = NULL;
	else
		rPosition = GetPosition(iter);
	return v;
}

template<class TYPE, class ARG_TYPE>
inline TYPE CList<TYPE, ARG_TYPE>::GetNext(POSITION& rPosition) const // return *Position++
{
	iterator iter = GetIterator(rPosition);
	TYPE v = *iter;
	++ iter;
	if (iter == _Self::end())
		rPosition = NULL;
	else
		rPosition = GetPosition(iter);
	return v;
}

template<class TYPE, class ARG_TYPE>
inline TYPE& CList<TYPE, ARG_TYPE>::GetPrev(POSITION& rPosition) // return *Position--
{
	iterator iter = GetIterator(rPosition);
	TYPE& v = *iter;
	if (_Self::begin() != iter)
	{
		rPosition = GetPosition(--iter);
	}
	else
		rPosition = NULL;
	return v;
}

template<class TYPE, class ARG_TYPE>
inline TYPE CList<TYPE, ARG_TYPE>::GetPrev(POSITION& rPosition) const // return *Position--
{
	iterator iter = GetIterator(rPosition);
	TYPE v = *iter;
	if (_Self::begin() != iter)
	{
		rPosition = GetPosition(--iter);
	}
	else
		rPosition = NULL;
	return v;
}

template<class TYPE, class ARG_TYPE>
inline TYPE& CList<TYPE, ARG_TYPE>::GetAt(POSITION position)
{
	assert(position);
	return *GetIterator(position);
}

template<class TYPE, class ARG_TYPE>
inline TYPE CList<TYPE, ARG_TYPE>::GetAt(POSITION position) const
{
	assert(position);
	return *GetIterator(position);
}

template<class TYPE, class ARG_TYPE>
inline void CList<TYPE, ARG_TYPE>::SetAt(POSITION pos, ARG_TYPE newElement)
{
	assert(pos);
	*GetIterator(pos) = newElement;
}

template<class TYPE, class ARG_TYPE>
CList<TYPE, ARG_TYPE>::CList()
{
}

template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::RemoveAll()
{
	_Self::clear();
}

template<class TYPE, class ARG_TYPE>
CList<TYPE, ARG_TYPE>::~CList()
{
	RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::AddHead(CList* pNewList)
{
	_Self::insert(_Self::begin(), pNewList->begin(), pNewList->end());
}

template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::AddTail(CList* pNewList)
{
	_Self::insert(_Self::end(), pNewList->begin(), pNewList->end());
}

template<class TYPE, class ARG_TYPE>
TYPE CList<TYPE, ARG_TYPE>::RemoveHead()
{
	TYPE data = _Self::front();
	_Self::pop_front();
	return data;
}

template<class TYPE, class ARG_TYPE>
TYPE CList<TYPE, ARG_TYPE>::RemoveTail()
{
	TYPE data = _Self::back();
	_Self::pop_back();
	return data;
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::AddHead(ARG_TYPE newElement)
{
	_Self::push_front(newElement);
	return GetPosition(_Self::begin());
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::AddTail(ARG_TYPE newElement)
{
	_Self::push_back(newElement);
	return GetPosition(-- _Self::end());
}


template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::RemoveAt(POSITION position)
{
	_Self::erase(GetIterator(position));
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::GetHeadPosition() const
{
	if (_Self::empty())
		return NULL;
	else
		return GetPosition(_Self::begin());
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::GetTailPosition() const
{
	if (_Self::empty())
		return NULL;
	return GetPosition(-- _Self::end());
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::InsertBefore(POSITION position, ARG_TYPE newElement)
{
	iterator iter = GetIterator(position);
	return GetPosition(_Self::insert(iter, newElement));
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::InsertAfter(POSITION position, ARG_TYPE newElement)
{
	if (position == NULL)
	{
		return AddTail(newElement);
	}
	else
	{
		return GetPosition(_Self::insert(++GetIterator(position), newElement));
	}
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::Find(ARG_TYPE searchValue, POSITION startAfter /*= NULL*/)
{
	iterator iter = startAfter ? GetIterator(startAfter) : _Self::begin();
	iterator find_iter = find(iter, _Self::end(), searchValue);
	if (find_iter == _Self::end())
		return NULL;
	else
		return GetPosition(find_iter);
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::FindIndex(int nIndex) const
{
	if (nIndex >= GetCount() || nIndex < 0)
		return NULL;
	const_iterator iter = _Self::begin();
	while(nIndex --)
		++ iter;
	return GetPosition(iter);
}

__NS_BASIC_END

#endif 
