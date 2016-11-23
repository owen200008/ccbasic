#pragma once
#ifndef BASIC_CONTAINER_H__
#define BASIC_CONTAINER_H__

#include "../inc/basic_def.h"
#include <utility>
#include <vector>
#include <list>
#include <deque>
#include <stack>
#include <queue>
#include <map>
#include <functional>
#include <set>

#ifdef __GNUC__
#include <string>
#include <ext/hash_map>
#include <ext/hash_set>
using namespace __gnu_cxx;
#elif defined(__SGI_SBASIC_PORT)
#include <string>
#include <hash_map>
#include <hash_set>
#else
#include <xstring>
#include <hash_map>
#include <hash_set>
#include <xhash>
using namespace stdext;
#endif

using namespace std;

__NS_BASIC_START

template<class _Kty>
struct basic_set
{
	typedef set<_Kty,	less<_Kty>, DEFAULT_ALLOCATOR<_Kty> >		type;
};

template<class _Kty>
struct basic_multiset
{
	typedef multiset<_Kty, less<_Kty>, DEFAULT_ALLOCATOR<_Kty> >		type;
};

template<class _Key, class _Tp>
struct basic_map
{
	typedef map<_Key, _Tp, less<_Key>, DEFAULT_ALLOCATOR<_Tp> >			type;
};

template<class _Key, class _Tp>
struct basic_multimap
{
	typedef multimap<_Key, _Tp, less<_Key>, DEFAULT_ALLOCATOR<_Tp> >	type;
};

#if defined(__GNUC__) || defined (__SGI_SBASIC_PORT)

template<class _Key>
struct basic_hash_set
{
    typedef hash_set<_Key, __gnu_cxx::hash<_Key>, equal_to<_Key>, DEFAULT_ALLOCATOR<_Key> >	type;
};

template<class _Key, class _Tp>
struct basic_hash_multiset
{
	typedef hash_multiset<_Key, __gnu_cxx::hash<_Key>, equal_to<_Key>, DEFAULT_ALLOCATOR<_Key> >	type;
};

template<class _Key, class _Tp>
struct basic_hash_map
{
	typedef hash_map<_Key, _Tp, __gnu_cxx::hash<_Key>, equal_to<_Key>, DEFAULT_ALLOCATOR<_Tp> >	type;
};

template<class _Key, class _Tp>
struct basic_hash_multimap
{
	typedef hash_multimap<_Key, _Tp, __gnu_cxx::hash<_Key>, equal_to<_Key>, DEFAULT_ALLOCATOR<_Tp> > type;
};

#else

template<class _Kty, class _Ty>
struct basic_hash_map
{
	typedef hash_map<_Kty, _Ty, hash_compare<_Kty, less<_Kty> >, DEFAULT_ALLOCATOR<pair<const _Kty, _Ty> > >	type;
};

template<class _Kty, class _Ty>
struct basic_hash_multimap
{
	typedef hash_multimap<_Kty, _Ty, hash_compare<_Kty, less<_Kty> >, DEFAULT_ALLOCATOR<pair<const _Kty, _Ty> > >	type;
};

template<class _Kty>
struct basic_hash_set
{
	typedef hash_set<_Kty, hash_compare<_Kty, less<_Kty> >, DEFAULT_ALLOCATOR<_Kty> >		type;
};

template<class _Kty>
struct basic_hash_multiset
{
	typedef hash_multiset<_Kty, hash_compare<_Kty, less<_Kty> >, DEFAULT_ALLOCATOR<_Kty> >		type;
};

#endif

template<class _Ty>
struct basic_vector
{
	typedef vector<_Ty, DEFAULT_ALLOCATOR<_Ty> >	type;
};

template<class _Ty>
struct basic_list
{
	typedef list<_Ty, DEFAULT_ALLOCATOR<_Ty> >	type;
};

template<class _Ty>
struct basic_deque
{
	typedef deque<_Ty, DEFAULT_ALLOCATOR<_Ty> >	type;
};

template<class _Ty>
struct basic_stack
{
	typedef typename basic_deque<_Ty>::type SequenceType;
	typedef stack<_Ty, SequenceType>	type;
};

template<class _Ty>
struct basic_queue
{
	typedef typename basic_deque<_Ty>::type SequenceType;
	typedef queue<_Ty, SequenceType>	type;
};


template<class _Ty>
struct basic_priority_queue
{
	typedef priority_queue<_Ty, basic_vector<_Ty> >	type;
};

template<class _Elem>
struct basic_basic_string
{
	typedef basic_string<_Elem, std::char_traits<_Elem>, DEFAULT_ALLOCATOR<_Elem> >	type;
};

template<class _Elem>
struct basic_basic_stringstream
{
	typedef basic_stringstream<_Elem, std::char_traits<_Elem>, DEFAULT_ALLOCATOR<_Elem> >	type;
};


__NS_BASIC_END

#endif // BASIC_CONTAINER_H__
