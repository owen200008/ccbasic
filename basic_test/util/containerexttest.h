#ifndef INC_CONTAINEXTTEST_H
#define INC_CONTAINEXTTEST_H

#include <basic.h>
#include "../headdefine.h"

#pragma	pack(1)
struct ctx_message
{
	uint32_t		m_nCtxID;
	int32_t			m_session;
	uint32_t		m_nType;
	void*			m_data;
	size_t			sz;
	ctx_message(){
		memset(this, 0, sizeof(ctx_message));
	}
	ctx_message(ctx_message&& msg){
		*this = msg;
		msg.sz = 0;
	}
	~ctx_message(){
		if (sz > 0){
			basiclib::BasicDeallocate(m_data);
		}
	}
	ctx_message& operator = (ctx_message&& msg){
		*this = msg;
		msg.sz = 0;
		return *this;
	}
};
#pragma	pack()


LONG g_pushtimes = 0;
LONG g_poptimes = 0;
LONG g_addC = 0;
LONG g_delC = 0;
ctx_message msg;
ctx_message* GetPushCtx(){
	LONG lRet = basiclib::BasicInterlockedIncrement(&g_pushtimes);
	if (lRet > TIMES_FAST)
		return nullptr;
	return &msg;
}
/*
ctx_message* GetPopCtx(){
	LONG lRet = basiclib::BasicInterlockedIncrement(&g_poptimes);
	if (lRet > TIMES_FAST)
		return nullptr;
	return &msg;
}*/

basiclib::CMessageQueueLock<ctx_message> msgQueue;
THREAD_RETURN CMessageQueueThreadPush(void* arg){
	clock_t begin = clock();
	int nIndex = 0;
	while (true){
		ctx_message* pRet = GetPushCtx();
		if (pRet == nullptr)
			break;
		msgQueue.MQPush(&msg);
		nIndex++;
	}
	clock_t end = clock();
	g_addC += end - begin;
	printf("SpinLockMQPush(%d) %d\n", nIndex, end - begin);
	return 0;
}
THREAD_RETURN CMessageQueueThreadPop(void* arg){
	clock_t begin = clock();
	int nIndex = 0;
	while (true){
		if (msgQueue.MQPop(&msg)){
			break;
		}
		nIndex++;
	}
	clock_t end = clock();
	g_delC += end - begin;
	printf("SpinLockMQPop(%d) %d\n", nIndex, end - begin);
	return 0;
}

basiclib::CLockFreeMessageQueue<ctx_message> conMsgQueue;
THREAD_RETURN ConcurrentQueueThreadPush(void* arg){
	clock_t begin = clock();
	int nIndex = 0;
	while (true){
		ctx_message* pRet = GetPushCtx();
		if (pRet == nullptr)
			break;
		moodycamel::ProducerToken token(conMsgQueue);
		conMsgQueue.enqueue(token, std::move(*pRet));
		nIndex++;
	}
	clock_t end = clock();
	g_addC += end - begin;
	printf("LockFreeMQPush(%d) %d\n", nIndex, end - begin);
	return 0;
}
THREAD_RETURN ConcurrentQueueThreadPushToken(void* arg){
	moodycamel::ProducerToken token(conMsgQueue);
	clock_t begin = clock();
	int nIndex = 0;
	while (true){
		ctx_message* pRet = GetPushCtx();
		if (pRet == nullptr)
			break;
		conMsgQueue.enqueue(token, std::move(*pRet));
		nIndex++;
	}
	clock_t end = clock();
	g_addC += end - begin;
	printf("LockFreeMQPushToken(%d) %d\n", nIndex, end - begin);
	return 0;
}
THREAD_RETURN ConcurrentQueueThreadPop(void* arg){
	clock_t begin = clock();
	int nIndex = 0;
	while (true){
		moodycamel::ConsumerToken tokenCon(conMsgQueue);
		if (!conMsgQueue.try_dequeue(tokenCon, msg)){
			break;
		}
		nIndex++;
	}
	clock_t end = clock();
	g_delC += end - begin;
	printf("LockFreeMQPop(%d) %d\n", nIndex, end - begin);
	return 0;
}
THREAD_RETURN ConcurrentQueueThreadPopToken(void* arg){
	moodycamel::ConsumerToken tokenCon(conMsgQueue);
	clock_t begin = clock();
	int nIndex = 0;
	while (true){
		if (!conMsgQueue.try_dequeue(msg)){
			break;
		}
		nIndex++;
	}
	clock_t end = clock();
	g_delC += end - begin;
	printf("LockFreeMQPopToken(%d) %d\n", nIndex, end - begin);
	return 0;
}

#define CREATE_THREAD 8
HANDLE g_thread[CREATE_THREAD];
void TestContainExt()
{
	
	{
		for (int j = 0; j < 5; j++)
		{
			clock_t begin = clock();
			for (int i = 0; i < TIMES_FAST; i++){
				moodycamel::ConsumerToken tokenCon(conMsgQueue);
				conMsgQueue.try_dequeue(tokenCon, msg);
			}
			clock_t end = clock();
			printf("dequeue Total Times %d\n", end - begin);
		}
		for (int j = 0; j < 5; j++)
		{
			clock_t begin = clock();
			for (int i = 0; i < TIMES_FAST; i++)
				msgQueue.MQPop(&msg);
			clock_t end = clock();
			printf("dequeue Total Times %d\n", end - begin);
		}
	}

	g_addC = 0;
	g_delC = 0;
	DWORD dwThreadServerID = 0;
	if (true)
	{
		if (CREATE_THREAD <= 4){
			clock_t begin = clock();
			for (int i = 0; i < 5; i++)
			{
				g_pushtimes = 0;
				g_poptimes = 0;
				for (int j = 0; j < CREATE_THREAD; j++)
				{
					g_thread[j] = basiclib::BasicCreateThread(CMessageQueueThreadPush, nullptr, &dwThreadServerID);
				}
				for (int j = 0; j < CREATE_THREAD; j++)
				{
					basiclib::BasicWaitForSingleObject(g_thread[j], -1);
				}
				for (int j = 0; j < CREATE_THREAD; j++)
				{
					g_thread[j] = basiclib::BasicCreateThread(CMessageQueueThreadPop, nullptr, &dwThreadServerID);
				}
				for (int j = 0; j < CREATE_THREAD; j++)
				{
					basiclib::BasicWaitForSingleObject(g_thread[j], -1);
				}
			}
			clock_t end = clock();
			printf("Total MQPush %d\n", g_addC);
			printf("Total MQPop %d\n", g_delC);
			printf("Total Times %d\n", end - begin);
		}
	}
	if (true)
	{
		g_addC = 0;
		g_delC = 0;
		clock_t begin = clock();
		for (int i = 0; i < 5; i++)
		{
			g_pushtimes = 0;
			g_poptimes = 0;
			for (int j = 0; j < CREATE_THREAD; j++)
			{
				g_thread[j] = basiclib::BasicCreateThread(ConcurrentQueueThreadPushToken, nullptr, &dwThreadServerID);
			}
			for (int j = 0; j < CREATE_THREAD; j++)
			{
				basiclib::BasicWaitForSingleObject(g_thread[j], -1);
			}
			for (int j = 0; j < CREATE_THREAD; j++)
			{
				g_thread[j] = basiclib::BasicCreateThread(ConcurrentQueueThreadPopToken, nullptr, &dwThreadServerID);
			}
			for (int j = 0; j < CREATE_THREAD; j++)
			{
				basiclib::BasicWaitForSingleObject(g_thread[j], -1);
			}
		}
		clock_t end = clock();
		printf("Total MQPush %d\n", g_addC);
		printf("Total MQPop %d\n", g_delC);
		printf("Total Times %d\n", end - begin);
	}
	{
		for (int j = 0; j < 5; j++)
		{
			clock_t begin = clock();
			for (int i = 0; i < TIMES_FAST; i++)
				conMsgQueue.try_dequeue(msg);
			clock_t end = clock();
			printf("dequeue Total Times %d\n", end - begin);
		}
	}
	if (true)
	{
		g_addC = 0;
		g_delC = 0;
		clock_t begin = clock();
		for (int i = 0; i < 5; i++)
		{
			g_pushtimes = 0;
			g_poptimes = 0;
			for (int j = 0; j < CREATE_THREAD; j++)
			{
				g_thread[j] = basiclib::BasicCreateThread(ConcurrentQueueThreadPush, nullptr, &dwThreadServerID);
			}
			for (int j = 0; j < CREATE_THREAD; j++)
			{
				basiclib::BasicWaitForSingleObject(g_thread[j], -1);
			}
			for (int j = 0; j < CREATE_THREAD; j++)
			{
				g_thread[j] = basiclib::BasicCreateThread(ConcurrentQueueThreadPop, nullptr, &dwThreadServerID);
			}
			for (int j = 0; j < CREATE_THREAD; j++)
			{
				basiclib::BasicWaitForSingleObject(g_thread[j], -1);
			}
		}
		clock_t end = clock();
		printf("Total MQPush %d\n", g_addC);
		printf("Total MQPop %d\n", g_delC);
		printf("Total Times %d\n", end - begin);
	}
	{
		for (int j = 0; j < 5; j++)
		{
			clock_t begin = clock();
			for (int i = 0; i < TIMES_FAST; i++)
				conMsgQueue.try_dequeue(msg);
			clock_t end = clock();
			printf("dequeue Total Times %d\n", end - begin);
		}
	}
}


#endif