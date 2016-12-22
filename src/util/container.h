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
#include <unordered_map>
#include <unordered_set>
#include <functional>

using namespace std;

__NS_BASIC_START

template<class _Kty>
struct basic_set
{
	typedef unordered_set<_Kty, hash<_Kty>, equal_to<_Kty>, DEFAULT_ALLOCATOR<_Kty> >		type;
};

template<class _Kty>
struct basic_multiset
{
	typedef unordered_multiset<_Kty, hash<_Kty>, equal_to<_Kty>, DEFAULT_ALLOCATOR<_Kty> >		type;
};

template<class _Key, class _Tp>
struct basic_map
{
	typedef unordered_map<_Key, _Tp, hash<_Key>, equal_to<_Key>, DEFAULT_ALLOCATOR<_Tp> >			type;
};

template<class _Key, class _Tp>
struct basic_order_map
{
	typedef map<_Key, _Tp, less<_Key>, DEFAULT_ALLOCATOR<_Tp> >			type;
};

template<class _Key, class _Tp>
struct basic_multimap
{
	typedef unordered_multimap<_Key, _Tp, hash<_Key>, equal_to<_Key>, DEFAULT_ALLOCATOR<_Tp> >	type;
};

template<class _Key, class _Tp>
struct basic_order_multimap
{
	typedef multimap<_Key, _Tp, less<_Key>, DEFAULT_ALLOCATOR<_Tp> >	type;
};

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
