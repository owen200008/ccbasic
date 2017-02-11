#ifndef INC_CONTAINEXTTEST_H
#define INC_CONTAINEXTTEST_H

#include <basic.h>

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

#define TIMES 10000000
LONG g_pushtimes = 0;
LONG g_poptimes = 0;
LONG g_addC = 0;
LONG g_delC = 0;
ctx_message msg;
ctx_message* GetPushCtx(){
	LONG lRet = basiclib::BasicInterlockedIncrement(&g_pushtimes);
	if (lRet > TIMES)
		return nullptr;
	return &msg;
}
ctx_message* GetPopCtx(){
	LONG lRet = basiclib::BasicInterlockedIncrement(&g_poptimes);
	if (lRet > TIMES)
		return nullptr;
	return &msg;
}

basiclib::CMessageQueueLock<ctx_message> msgQueue;
THREAD_RETURN CMessageQueueThreadPush(void* arg){
	clock_t begin = clock();
	while (true){
		ctx_message* pRet = GetPushCtx();
		if (pRet == nullptr)
			break;
		msgQueue.MQPush(&msg);
	}
	clock_t end = clock();
	g_addC += end - begin;
	printf("MQPush %d\n", end - begin);
	return 0;
}
THREAD_RETURN CMessageQueueThreadPop(void* arg){
	clock_t begin = clock();
	while (true){
		ctx_message* pRet = GetPopCtx();
		if (pRet == nullptr)
			break;
		msgQueue.MQPop(&msg);
	}
	clock_t end = clock();
	g_delC += end - begin;
	printf("MQPop %d\n", end - begin);
	return 0;
}

basiclib::CLockFreeMessageQueue<ctx_message> conMsgQueue;
THREAD_RETURN ConcurrentQueueThreadPush(void* arg){
	clock_t begin = clock();
	while (true){
		ctx_message* pRet = GetPushCtx();
		if (pRet == nullptr)
			break;
		conMsgQueue.enqueue(std::move(*pRet));
	}
	clock_t end = clock();
	g_addC += end - begin;
	printf("MQPush %d\n", end - begin);
	return 0;
}
THREAD_RETURN ConcurrentQueueThreadPop(void* arg){
	clock_t begin = clock();
	while (true){
		ctx_message* pRet = GetPopCtx();
		if (pRet == nullptr)
			break;
		conMsgQueue.try_dequeue(*pRet);
	}
	clock_t end = clock();
	g_delC += end - begin;
	printf("MQPop %d\n", end - begin);
	return 0;
}

#define CREATE_THREAD 1
HANDLE g_thread[CREATE_THREAD];
void TestContainExt()
{
	{
		ctx_message msg2;
		msg2.m_data = basiclib::BasicAllocate(1024);
		msg2.sz = 1024;
		conMsgQueue.enqueue(std::move(msg2));
	}
	{
		ctx_message msg2;
		conMsgQueue.try_dequeue(msg2);
		if (msg2.sz > 0){

		}
	}

	g_addC = 0;
	g_delC = 0;
	DWORD dwThreadServerID = 0;
	if (true)
	{
		clock_t begin = clock();
		for (int i = 0; i < 10; i++)
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
	if (true)
	{
		g_addC = 0;
		g_delC = 0;
		clock_t begin = clock();
		for (int i = 0; i < 10; i++)
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


	
}


#endif