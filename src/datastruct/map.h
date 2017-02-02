/***********************************************************************************************
// 文件名:     map.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 9:16:27
// 内容描述:   实现map的MFC like接口。仅用于兼容，不建议使用。
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_MAP_H
#define BASIC_MAP_H

#pragma once

//#include "../strutil/strutil.h"
#include "../inc/basic_def.h"
#include <utility>
#include <functional>
#include "../util/container.h"

#include "../types/types.h"
using namespace std;

__NS_BASIC_START

#define BEFORE_START_POSITION ((POSITION)-1L)

/////////////////////////////////////////////////////////////////////////////
// CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>
#define base_class basic_map<KEY, VALUE>

#pragma warning (push)
#pragma warning (disable: 4251)
#pragma warning (disable: 4275)

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
class CMap : public base_class
{
public:
	typedef typename base_class::iterator iterator;
	typedef typename base_class::const_iterator const_iterator;
	typedef CMap<KEY, ARG_KEY, VALUE, ARG_VALUE> _Self;
	typedef pair<const KEY, VALUE>	pair_type;
// Construction
	CMap(int nBlockSize = 10);

// Attributes
	//! number of elements
	int GetCount() const;
	//! if is empty
	BOOL IsEmpty() const;

	//! get value by key
	BOOL Lookup(ARG_KEY key, VALUE& rValue) const;

	//! add a new (key, value) pair
	void SetAt(ARG_KEY key, ARG_VALUE newValue);

	//! removing existing (key, ?) pair
	BOOL RemoveKey(ARG_KEY key);

	//! removing all existing pair
	void RemoveAll();

	// iterating all (key, value) pairs
	//! get start position
	POSITION GetStartPosition() const;
	//! get ++rNextPosition element
	void GetNextAssoc(POSITION& rNextPosition, KEY& rKey, VALUE& rValue) const;

	// advanced features for derived classes
	//! get hash table size
	UINT GetHashTableSize() const;
	//! set hash table size, for MFC compatible, not really implement
	void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

// Implementation
protected:

public:
	~CMap();
//protected:

#if defined(_MSC_VER) && !defined(__SGI_SBASIC_PORT)
	typedef typename base_class::_Mylist _base_list;
	class _wrap_list_helper : public _base_list
	{
	public:
		typedef typename _base_list::_Nodeptr _Nodeptr;
	};

	POSITION GetPosition(const_iterator iter) const
	{
		return (POSITION)iter._Mynode();
	}
	POSITION GetPosition(iterator iter) const
	{
		return (POSITION)iter._Mynode();
	}

	iterator GetIterator(POSITION position) const
	{
		typedef _wrap_list_helper::_Nodeptr _Nodeptr;
#if _HAS_ITERATOR_DEBUGGING || _SECURE_SCL
		return iterator((_Nodeptr)position, &_List);
#else // _HAS_ITERATOR_DEBUGGING 
		return iterator((_Nodeptr)position);
#endif	// _HAS_ITERATOR_DEBUGGING 
	}


	const_iterator GetConstIterator(POSITION position) const
	{		
		typedef _wrap_list_helper::_Nodeptr _Nodeptr;
#if _HAS_ITERATOR_DEBUGGING || _SECURE_SCL
		return const_iterator((_Nodeptr)position, &_List);
#else // _HAS_ITERATOR_DEBUGGING 
		return const_iterator((_Nodeptr)position);
#endif	// _HAS_ITERATOR_DEBUGGING 
	}
#else // __MSC_VER
	POSITION GetPosition(const_iterator iter) const
	{
		return (POSITION)iter._M_cur;
	}
	
	POSITION GetPosition(iterator iter) const
	{
		return (POSITION)iter._M_cur;
	}

	iterator GetIterator(POSITION position) 
	{
		typedef typename iterator::_Node* _Nodeptr;
		return iterator((_Nodeptr)position, _Self::begin()._M_ht);
	}

	const_iterator GetConstIterator(POSITION position) const
	{	
		typedef typename iterator::_Node* _Nodeptr;
		return const_iterator((_Nodeptr)position, _Self::begin()._M_ht);
	}
#endif // __MSC_VER
};

#pragma warning (pop)
/////////////////////////////////////////////////////////////////////////////
// CMap<KEY, ARG_KEY, VALUE, ARG_VALUE> inline functions

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
inline int CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetCount() const
{
	return _Self::size();
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
inline BOOL CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::IsEmpty() const
{
	return _Self::empty();
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
inline void CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::SetAt(ARG_KEY key, ARG_VALUE newValue)
{
	(*this)[key] = newValue;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
inline POSITION CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetStartPosition() const
{
	return _Self::empty() ? NULL : GetPosition(_Self::begin());
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
inline UINT CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetHashTableSize() const
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CMap<KEY, ARG_KEY, VALUE, ARG_VALUE> out-of-line functions

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::CMap(int nBlockSize)
{
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::InitHashTable(
	UINT nHashSize, BOOL bAllocNow)
//
// Used to force allocation of a hash table or to override the default
//   hash table size of (which is fairly small)
{
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::RemoveAll()
{
	_Self::clear();
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::~CMap()
{
	_Self::clear();
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
BOOL CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::Lookup(ARG_KEY key, VALUE& rValue) const
{
	const_iterator iter = _Self::find(key);
	if (_Self::end() == iter)
	{
		return FALSE;
	}
	else
	{
		rValue = iter->second;
		return TRUE;
	}
	
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
BOOL CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::RemoveKey(ARG_KEY key)
// remove key - return TRUE if removed
{
	return _Self::erase(key) > 0;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetNextAssoc(POSITION& rNextPosition,
	KEY& rKey, VALUE& rValue) const
{
	assert(rNextPosition != NULL);
	const_iterator iter = GetConstIterator(rNextPosition);
	rKey	= iter->first;
	rValue	= iter->second;
	rNextPosition = ++ iter == _Self::end() ? NULL : GetPosition(iter);
}

__NS_BASIC_END

#endif 
