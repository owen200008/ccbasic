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
class basic_set : public unordered_set<_Kty, hash<_Kty>, equal_to<_Kty>, DEFAULT_ALLOCATOR<_Kty> >
{
};

template<class _Kty>
class basic_multiset : public unordered_multiset<_Kty, hash<_Kty>, equal_to<_Kty>, DEFAULT_ALLOCATOR<_Kty> >
{
};

template<class _Key, class _Tp>
class basic_map : public unordered_map<_Key, _Tp, hash<_Key>, equal_to<_Key>, DEFAULT_ALLOCATOR<pair<const _Key, _Tp>>>
{
};

template<class _Key, class _Tp>
class basic_order_map : public map<_Key, _Tp, less<_Key>, DEFAULT_ALLOCATOR<pair<const _Key, _Tp>> >
{
};

template<class _Key, class _Tp>
class basic_multimap : public unordered_multimap<_Key, _Tp, hash<_Key>, equal_to<_Key>, DEFAULT_ALLOCATOR<pair<const _Key, _Tp>> >
{
};

template<class _Key, class _Tp>
class basic_order_multimap : public multimap<_Key, _Tp, less<_Key>, DEFAULT_ALLOCATOR<pair<const _Key, _Tp>> >
{
};

template<class _Ty>
class basic_vector : public vector<_Ty, DEFAULT_ALLOCATOR<_Ty> >
{
};

template<class _Ty>
class basic_list : public list<_Ty, DEFAULT_ALLOCATOR<_Ty> >
{
};

template<class _Ty>
class basic_deque : public deque<_Ty, DEFAULT_ALLOCATOR<_Ty> >
{
};

template<class _Ty>
class basic_stack : public stack<_Ty, basic_deque<_Ty>>
{
};


template<class _Ty>
class basic_queue : public queue<_Ty, basic_deque<_Ty>>
{
};


template<class _Ty>
struct basic_priority_queue : public priority_queue<_Ty, basic_vector<_Ty> >
{
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
