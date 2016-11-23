#pragma once
#ifndef BASIC_CONTAINEREXT_H__
#define BASIC_CONTAINEREXT_H__

#include "../inc/basic.h"

__NS_BASIC_START


#define DEFAULT_QUEUE_SIZE			64
#define DEFAULT_QUEUE_OVERLOAD		1024

template<class StructData>
class CMessageQueue : public basiclib::CBasicObject
{
public:
	typedef fastdelegate::FastDelegate1<int, void> OverLoadLengthCallback;
	typedef fastdelegate::FastDelegate2<StructData*, void*, void> DefaultReleaseFunc;

	CMessageQueue(int nDefaultQueueSize = DEFAULT_QUEUE_SIZE, int nOverLoadLength = DEFAULT_QUEUE_OVERLOAD){
		m_cap = nDefaultQueueSize;
		m_head = 0;
		m_tail = 0;
		m_overload_threshold = nOverLoadLength;
		m_defaultoverload = nOverLoadLength;
		m_overloadCallback = nullptr;
		m_defaultReleaseFunc = nullptr;
		m_queue = (StructData*)basiclib::BasicAllocate(m_cap * sizeof(StructData));
	}
	virtual ~CMessageQueue(){
		if (m_defaultReleaseFunc != nullptr)
			Drop_Queue(m_defaultReleaseFunc, nullptr);
		basiclib::BasicDeallocate(m_queue);
	}
	void SetOverLoadLength(int nOverLoadLength){
		m_overload_threshold = nOverLoadLength;
		m_defaultoverload = nOverLoadLength;
	}
	bool SetOverLoadCallbackFunc(OverLoadLengthCallback& func){m_overloadCallback = func;}
	bool SetDefaultStructReleaseFunc(const std::function<void(StructData *, void *)>& func){m_defaultReleaseFunc = func;}
	//插入包
	virtual void MQPush(StructData* message){
		m_queue[m_tail] = *message;
		if (++m_tail >= m_cap)
		{
			m_tail = 0;
		}
		if (m_head == m_tail)
		{
			expand_queue();
		}
	}
	//获取包,0代表成功，1代表没有
	virtual int MQPop(StructData* message){
		int ret = 1;
		if (m_head != m_tail)
		{
			*message = m_queue[m_head++];
			ret = 0;

			if (m_head >= m_cap)
			{
				m_head = 0;
			}
			int length = m_tail - m_head;
			if (length < 0)
			{
				length += m_cap;
			}
			while (length > m_overload_threshold)
			{
				if (m_overloadCallback != nullptr)
					m_overloadCallback(m_overload_threshold);
				m_overload_threshold *= 2;
			}
		}
		else
		{
			// reset overload_threshold when queue is empty
			m_overload_threshold = m_defaultoverload;
		}
		return ret;
	}

	//获取个数
	virtual int GetMQLength(){
		int head, tail, cap;
		head = m_head;
		tail = m_tail;
		cap = m_cap;
		if (head <= tail)
			return tail - head;
		return tail + cap - head;
	}

	//清空队列，依次回调
	void Drop_Queue(const std::function<void(StructData *, void *)>& func, void *ud){
		StructData msg;
		while (!MQPop(&msg))
		{
			func(&msg, ud);
		}
	}
	//直接清空队列
	void ClearQueue()
	{
		if (m_defaultReleaseFunc)
		{
			Drop_Queue(m_defaultReleaseFunc, nullptr);
		}
		else
		{
			m_head = 0;
			tail = 0;
		}
	}
protected:
	void expand_queue(){
		StructData* new_queue = (StructData*)basiclib::BasicAllocate(m_cap * 2 * sizeof(StructData));
		int i;
		for (i = 0; i<m_cap; i++)
		{
			new_queue[i] = m_queue[(m_head + i) % m_cap];
		}
		m_head = 0;
		m_tail = m_cap;
		m_cap *= 2;
		basiclib::BasicDeallocate(m_queue);
		m_queue = new_queue;
		TRACE("expand_queue:%d\n", m_cap * 2 * sizeof(StructData));
	}
protected:
	int												m_cap;
	int												m_head;
	int												m_tail;
	int 											m_overload_threshold;
	int												m_defaultoverload;

	StructData*										m_queue;
	OverLoadLengthCallback							m_overloadCallback;
	DefaultReleaseFunc								m_defaultReleaseFunc;
};

template<class StructData>
class CMessageQueueLock : public CMessageQueue<StructData>
{
public:
	CMessageQueueLock(int nDefaultQueueSize = DEFAULT_QUEUE_SIZE, int nOverLoadLength = DEFAULT_QUEUE_OVERLOAD) :
		CMessageQueue<StructData>(nDefaultQueueSize, nOverLoadLength){

	}
	virtual ~CMessageQueueLock(){

	}
	virtual void MQPush(StructData* message){
		basiclib::CSpinLockFunc lock(&m_lock, TRUE);
		CMessageQueue<StructData>::MQPush(message);
	}
	virtual int MQPop(StructData* message){
		basiclib::CSpinLockFunc lock(&m_lock, TRUE);
		return CMessageQueue<StructData>::MQPop(message);
	}
	virtual int GetMQLength(){
		basiclib::CSpinLockFunc lock(&m_lock, TRUE);
		return CMessageQueue<StructData>::GetMQLength();
	}

protected:
	basiclib::SpinLock		m_lock;
};

///////////////////////////////////////////////////////////////////////////////////////
//内存检测不能用，因为不能重复调用allocate
//stack，dbghelp决定必须单线程
template<class KeyType>
class CCheckNoPairKey : public basiclib::CBasicObject
{
public:
	typedef typename basiclib::basic_map<KeyType, stacktrace::call_stack>::type MapPair;
	CCheckNoPairKey(){

	}
	virtual ~CCheckNoPairKey(){

	}
	void Add(KeyType& value){
		basiclib::CSpinLockFunc lock(&m_spinlock, TRUE);
		stacktrace::call_stack stack(0);
		m_map[value].SwapStack(stack);
	}
	void Del(KeyType& value){
		basiclib::CSpinLockFunc lock(&m_spinlock, TRUE);
		m_map.erase(value);
		lock.UnLock();
	}
	void Dump(const std::function<void(KeyType, stacktrace::call_stack&)>& func){
		basiclib::CSpinLockFunc lock(&m_spinlock, TRUE);
		for (auto& checkData : m_map){
			func(checkData.first, checkData.second);
		}
	}
protected:
	MapPair		m_map;
	SpinLock	m_spinlock;
};

__NS_BASIC_END

#endif // BASIC_CONTAINER_H__
