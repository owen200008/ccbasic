#ifndef INC_CONTAINEXTTEST_H
#define INC_CONTAINEXTTEST_H

#include <basic.h>
#include "../headdefine.h"

//必须是2的指数幂 BASICQUEUE_MAX_ALLOCTIMES=最大分配次数 BASICQUEUE_ALLOCMULTYTIMES=每次分配增大的内存量 BASICQUEUE_DELETEQUEUESIZE=如果分配的数量大于4096*BASICQUEUE_ALLOCMULTYTIME*BASICQUEUE_ALLOCMULTYTIMES回收小于等于这个范围的
//默认值 第一次分配16 第二次分配64 第三次分配256 第四次分配1024 以此类推到16次
template<class T, uint32_t BASICQUEUE_MAX_ALLOCTIMES = 16, uint32_t BASICQUEUE_ALLOCMULTYTIMES = 4, uint32_t BASICQUEUE_DELETEQUEUESIZE = 16384>
class CBasicQueueArray : public basiclib::CBasicObject
{
public:
	struct ArrayNode{
		T										m_data;
		std::atomic<bool>						m_bReadAlready;
	};

	class AllocateIndexData : public basiclib::CBasicObject
	{
	public:
		inline uint32_t GetDataIndex(uint32_t uValue){
			return uValue % m_nMaxCount;
		}
		AllocateIndexData(int nCount){
			m_nMaxCount = nCount;
			m_nRead = 0;
			m_nPreWrite = 0;
			m_pPool = (ArrayNode*)basiclib::BasicAllocate(sizeof(ArrayNode) * nCount);
			memset(m_pPool, 0, sizeof(ArrayNode) * nCount);
		}
		virtual ~AllocateIndexData(){
			if (m_pPool){
				basiclib::BasicDeallocate(m_pPool);
			}
		}
		bool Push(const T& value){
			uint32_t nWriteIndex = m_nPreWrite.load(std::memory_order_relaxed);
			uint32_t nReadIndex = 0;
			do{
				nReadIndex = m_nRead.load(std::memory_order_relaxed);
				if (GetDataIndex(nWriteIndex + 1) == GetDataIndex(nReadIndex)){
					//full
					return false;
				}
			} while (!m_nPreWrite.compare_exchange_weak(nWriteIndex, nWriteIndex + 1, std::memory_order_release, std::memory_order_relaxed));

			ArrayNode& node = m_pPool[GetDataIndex(nWriteIndex)];
			node.m_data = value;
			node.m_bReadAlready.store(true);
			return true;
		}
		bool Pop(T& value, uint32_t& nIndex){
			uint32_t nReadIndex = m_nRead.load(std::memory_order_relaxed);
			ArrayNode* pNode = nullptr;  
			do{
				pNode = &(m_pPool[GetDataIndex(nReadIndex)]);
				if (!pNode->m_bReadAlready.load(std::memory_order_relaxed)){
					//empty
					if (m_nRead.compare_exchange_weak(nReadIndex, nReadIndex)){
						return false;
					}
					//这边continue后面必须是true，如果直接compare_exchange_weak会导致pNode出错
					continue;
				}
				if (m_nRead.compare_exchange_weak(nReadIndex, nReadIndex + 1, std::memory_order_release, std::memory_order_relaxed))
					break;

			} while (true);

			value = pNode->m_data;

			pNode->m_bReadAlready.store(false, std::memory_order_relaxed);
			return true;
		}
		bool IsEmpty(){
			uint32_t nReadIndex = m_nRead.load(std::memory_order_relaxed);
			do{
				ArrayNode* pNode = &(m_pPool[GetDataIndex(nReadIndex)]);
				if (!pNode->m_bReadAlready.load(std::memory_order_relaxed)){
					//empty
					if (m_nRead.compare_exchange_weak(nReadIndex, nReadIndex)){
						return true;
					}
					continue;
				}
				break;
			} while (true);
			return false;
		}
		uint32_t GetAllocCount(){ return m_nMaxCount; }
	public:
		std::atomic<uint32_t>	m_nRead;
		std::atomic<uint32_t>	m_nPreWrite;
		uint32_t				m_nMaxCount;
		ArrayNode*				m_pPool;
	};
public:
	inline uint32_t GetQueueArrayIndex(uint32_t nIndex){
		return nIndex % BASICQUEUE_MAX_ALLOCTIMES;
	}
	CBasicQueueArray(int nDefaultQueuePowerSize = 4){
		m_lock = 0;
		m_nNextQueueSize = (uint32_t)pow(2, nDefaultQueuePowerSize);
		m_cReadIndex = 0;
		m_cWriteIndex = 0;
		memset(m_pMaxAllocTimes, 0, BASICQUEUE_MAX_ALLOCTIMES * sizeof(AllocateIndexData*));

		m_pMaxAllocTimes[0] = new AllocateIndexData(m_nNextQueueSize);
		m_nNextQueueSize *= BASICQUEUE_ALLOCMULTYTIMES;
	}
	virtual ~CBasicQueueArray(){
		for (int i = 0; i <= BASICQUEUE_MAX_ALLOCTIMES; i++){
			if (m_pMaxAllocTimes[i])
				delete m_pMaxAllocTimes[i];
		}
	}
	bool Push(const T& value, int nDeep = 0){
		if (nDeep >= BASICQUEUE_MAX_ALLOCTIMES){
			ASSERT(0);
			return false;
		}
		uint32_t nGetWriteIndex = m_cWriteIndex.load(std::memory_order_relaxed);  
		uint32_t nWriteIndex = GetQueueArrayIndex(nGetWriteIndex);
		
		AllocateIndexData* pAllocData = m_pMaxAllocTimes[nWriteIndex];
		if (pAllocData){
			if (pAllocData->Push(value))
				return true;
			m_cWriteIndex.compare_exchange_weak(nGetWriteIndex, nGetWriteIndex + 1, std::memory_order_release, std::memory_order_relaxed);
			return Push(value, ++nDeep);
		}
		//自旋锁下面分配
		while (m_lock.exchange(1, std::memory_order_relaxed)){};
		if (m_pMaxAllocTimes[nWriteIndex]){
			m_lock.exchange(0, std::memory_order_relaxed);
			return Push(value, nDeep);
		}
		m_pMaxAllocTimes[nWriteIndex] = new AllocateIndexData(m_nNextQueueSize);
		m_nNextQueueSize *= BASICQUEUE_ALLOCMULTYTIMES;
		m_lock.exchange(0, std::memory_order_relaxed);
		TRACE("expand_queue:%d\n", m_nNextQueueSize / BASICQUEUE_ALLOCMULTYTIMES * sizeof(ArrayNode));
		return Push(value, nDeep);
	}
	bool Pop(T& value, uint32_t& nIndex){
		uint32_t nGetReadIndex = m_cReadIndex.load(std::memory_order_relaxed);
		uint32_t nReadIndex = GetQueueArrayIndex(nGetReadIndex);
		AllocateIndexData* pAllocData = m_pMaxAllocTimes[nReadIndex];
		if (!pAllocData){
			//非常特殊情况下才会进来，push当前的环写完切换到下一个环还没创建，这边进入pop才会进入这里
			return false;
		}
		if (pAllocData->Pop(value, nIndex)){
			return true;
		}

		//如果write跟read是同一个环就认为是空的
		uint32_t nWriteIndex = GetQueueArrayIndex(m_cWriteIndex.load(std::memory_order_relaxed));
		if (nWriteIndex == nReadIndex){
			//empty
			return false;
		}

		//不在同一个环，开始读下一个环
		if (m_cReadIndex.compare_exchange_weak(nGetReadIndex, nGetReadIndex + 1, std::memory_order_release, std::memory_order_relaxed)){
			//环数据读完，切换到下个环完成，迁移环
			uint32_t nFindBegin = nWriteIndex;
			uint32_t nFindIndex = 0;
			uint32_t nAllocCount = pAllocData->GetAllocCount();
			bool bDelete = false;
			if (nAllocCount <= BASICQUEUE_DELETEQUEUESIZE && nAllocCount < m_nNextQueueSize / pow(BASICQUEUE_ALLOCMULTYTIMES, 3)){
				bDelete = true;
			}
			while (m_lock.exchange(1, std::memory_order_relaxed)){};
		
			if (bDelete){
				m_pMaxAllocTimes[nReadIndex] = nullptr;
			}
			else{
				for (uint32_t i = 0; i < BASICQUEUE_MAX_ALLOCTIMES; i++){
					nFindIndex = GetQueueArrayIndex(nFindBegin + i);
					if (m_pMaxAllocTimes[nFindIndex] == nullptr){
						std::swap(m_pMaxAllocTimes[nReadIndex], m_pMaxAllocTimes[nFindIndex]);
						break;
					}
				}
			}
			m_lock.exchange(0, std::memory_order_relaxed);
			if (bDelete){
				//删除
				delete pAllocData;
			}
		}
		return Pop(value, nIndex);
	}
	bool IsEmpty(){
		uint32_t nReadIndex = GetQueueArrayIndex(m_cReadIndex.load(std::memory_order_relaxed));
		AllocateIndexData* pAllocData = m_pMaxAllocTimes[nReadIndex];
		if (!pAllocData){
			//非常特殊情况下才会进来，push当前的环写完切换到下一个环还没创建，这边进入pop才会进入这里
			return true;
		}
		if (!pAllocData->IsEmpty())
			return false;

		//如果write跟read是同一个环就认为是空的
		uint32_t nWriteIndex = GetQueueArrayIndex(m_cWriteIndex.load(std::memory_order_relaxed));
		if (nWriteIndex == nReadIndex){
			//empty
			return true;
		}
		return false;
	}
protected:
	uint32_t													m_nNextQueueSize;

	std::atomic<uint32_t>										m_cReadIndex;
	std::atomic<uint32_t>										m_cWriteIndex;

	std::atomic<char>											m_lock;
	AllocateIndexData*											m_pMaxAllocTimes[BASICQUEUE_MAX_ALLOCTIMES];
};

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
	ctx_message(uint32_t ctxid){
		memset(this, 0, sizeof(ctx_message));
		m_nCtxID = ctxid;
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
CBasicQueueArray<ctx_message> basicQueue;
THREAD_RETURN CBasicQueueThreadPush(void* arg){
	clock_t begin = clock();
	int nIndex = 0;
	while (true){
		ctx_message* pRet = GetPushCtx();
		if (pRet == nullptr)
			break;
		basicQueue.Push(msg);
		nIndex++;
	}
	clock_t end = clock();
	g_addC += end - begin;
	printf("BasicQueuePush(%d) %d\n", nIndex, end - begin);
	return 0;
}
THREAD_RETURN CBasicQueueThreadPop(void* arg){
	clock_t begin = clock();
	int nIndex = 0;
	uint32_t nIndex2 = 0;
	while (true){
		if (!basicQueue.Pop(msg, nIndex2)){
			break;
		}
		nIndex++;
	}
	clock_t end = clock();
	g_delC += end - begin;
	printf("BasicQueuePop(%d) %d\n", nIndex, end - begin);
	return 0;
}

LONG g_TotalPush = 0;
LONG g_TotalPop = 0;

const int g_CheckMaxValues = 100000000;
DWORD g_ayCheckMap[g_CheckMaxValues] = { 0 };
uint32_t g_ayCheckMapIndex[g_CheckMaxValues] = { 0 };
CBasicQueueArray<uint32_t> basicQueueInt;
THREAD_RETURN CBasicQueueThreadPush2(void* arg){
	clock_t begin = clock();
	int nIndex = 0;
	while (true){
		uint32_t lRet = basiclib::BasicInterlockedIncrement(&g_TotalPush);
		basicQueueInt.Push(lRet);
		if (lRet % (TIMES_FAST * 4) == 0){
			clock_t end = clock();
			printf("BasicQueuePush2(%d) %d empty(%d)\n", lRet, end - begin, basicQueueInt.IsEmpty());
		}
		if (g_TotalPush > g_TotalPop + TIMES_FAST * 4)
			basiclib::BasicSleep(1000);
	}
	return 0;
}
THREAD_RETURN CBasicQueueThreadPop2(void* arg){
	DWORD dwThreadID = basiclib::BasicGetCurrentThreadId();
	clock_t begin = clock();
	uint32_t nIndex = 0;
	uint32_t nGetIndex = 0;
	while (true){
		if (!basicQueueInt.Pop(nIndex, nGetIndex)){
			basiclib::BasicSleep(1);
			continue;
		}
		if (nIndex == 0){
			return 0;
		}
		uint32_t lRet = basiclib::BasicInterlockedIncrement(&g_TotalPop);
		if (g_ayCheckMap[nIndex % g_CheckMaxValues])
			return 0;
		if (g_TotalPop != nIndex)
			basiclib::BasicSleep(1);

		g_ayCheckMap[nIndex % g_CheckMaxValues] = dwThreadID;
		g_ayCheckMapIndex[nIndex % g_CheckMaxValues] = nGetIndex;
		
		if (lRet % (TIMES_FAST * 4) == 0){
			clock_t end = clock();
			printf("BasicQueuePop2(%d) %d empty(%d)\n", lRet, end - begin, basicQueueInt.IsEmpty());
		}
	}
	return 0;
}
THREAD_RETURN CBasicQueueThreadCheck(void* arg){
	uint32_t nCheckList = 1;
	while (true){
		if (!g_ayCheckMap[nCheckList % g_CheckMaxValues]){
			basiclib::BasicSleep(500);
			continue;
		}
			
		g_ayCheckMap[nCheckList % g_CheckMaxValues] = 0;
		nCheckList++;
		if (nCheckList % (TIMES_FAST * 4) == 0){
			clock_t end = clock();
			printf("CheckNumber(%d)\n", nCheckList);
		}
	}
	return 0;
}

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
		//moodycamel::ProducerToken token(conMsgQueue);
		conMsgQueue.enqueue(std::move(*pRet));
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
		//moodycamel::ConsumerToken tokenCon(conMsgQueue);
		if (!conMsgQueue.try_dequeue(msg)){
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
		if (!conMsgQueue.try_dequeue(tokenCon, msg)){
			break;
		}
		nIndex++;
	}
	clock_t end = clock();
	g_delC += end - begin;
	printf("LockFreeMQPopToken(%d) %d\n", nIndex, end - begin);
	return 0;
}

THREAD_RETURN TestSeq(void* arg){
	ctx_message* pMsg = (ctx_message*)arg;
	int nIndex = 0;
	for(int i = 0;i < 10;i++){
		{
			moodycamel::ProducerToken token(conMsgQueue);
			pMsg->m_session = i;
			if (!conMsgQueue.enqueue(token, *pMsg)){
				break;
			}
		}
		printf("TestSeq %d %d enqueue\n", pMsg->m_nCtxID, pMsg->m_session);
		basiclib::BasicSleep(1000);
	}
	return 0;
}
THREAD_RETURN TestSeq2(void* arg){
	ctx_message* pMsg = (ctx_message*)arg;
	int nIndex = 0;
	for (int i = 0; i < 10; i++){
		{
			moodycamel::ProducerToken token(conMsgQueue);
			pMsg->m_session = i * 2;
			if (!conMsgQueue.enqueue(*pMsg)){
				break;
			}
			printf("TestSeq %d %d enqueue\n", pMsg->m_nCtxID, pMsg->m_session);
			pMsg->m_session = i * 2 + 1;
			if (!conMsgQueue.enqueue(*pMsg)){
				break;
			}
		}
		printf("TestSeq %d %d enqueue\n", pMsg->m_nCtxID, pMsg->m_session);
		basiclib::BasicSleep(1000);
	}
	return 0;
}

#define CREATE_THREAD 8
HANDLE g_thread[CREATE_THREAD];
void TestContainExt()
{
	DWORD dwThreadServerID = 0;
	/*ctx_message data1(1), data2(2), data3(3);
	basiclib::BasicCreateThread(TestSeq, &data1, &dwThreadServerID);
	basiclib::BasicCreateThread(TestSeq2, &data2, &dwThreadServerID);
	basiclib::BasicCreateThread(TestSeq, &data3, &dwThreadServerID);
	basiclib::BasicSleep(1000);
	ctx_message data;
	while (conMsgQueue.try_dequeue(data)){
		printf("%d %d\n", data.m_nCtxID, data.m_session);
		basiclib::BasicSleep(1000);
	}
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
	}*/
	g_addC = 0;
	g_delC = 0;
	if (true)
	{
		clock_t begin = clock();
		for (int i = 0; i < 5; i++)
		{
			basiclib::BasicCreateThread(CBasicQueueThreadCheck, nullptr, &dwThreadServerID);
			g_pushtimes = 0;
			g_poptimes = 0;
			for (int j = 0; j < 1; j++)
			{
				g_thread[j] = basiclib::BasicCreateThread(CBasicQueueThreadPush2, nullptr, &dwThreadServerID);
			}
			for (int j = 0; j < CREATE_THREAD; j++)
			{
				g_thread[j] = basiclib::BasicCreateThread(CBasicQueueThreadPop2, nullptr, &dwThreadServerID);
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
	/*
	g_addC = 0;
	g_delC = 0;
	if (true)
	{
		clock_t begin = clock();
		for (int i = 0; i < 5; i++)
		{
			g_pushtimes = 0;
			g_poptimes = 0;
			for (int j = 0; j < CREATE_THREAD; j++)
			{
				g_thread[j] = basiclib::BasicCreateThread(CBasicQueueThreadPush, nullptr, &dwThreadServerID);
			}
			for (int j = 0; j < CREATE_THREAD; j++)
			{
				basiclib::BasicWaitForSingleObject(g_thread[j], -1);
			}
			for (int j = 0; j < CREATE_THREAD; j++)
			{
				g_thread[j] = basiclib::BasicCreateThread(CBasicQueueThreadPop, nullptr, &dwThreadServerID);
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
	g_addC = 0;
	g_delC = 0;
	if (true)
	{
		//if (CREATE_THREAD <= 4)
		{
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
	*/
	/*if (true)
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
	}*/
	/*if (true)
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
	}*/
	/*{
		for (int j = 0; j < 5; j++)
		{
			clock_t begin = clock();
			for (int i = 0; i < TIMES_FAST; i++)
				conMsgQueue.try_dequeue(msg);
			clock_t end = clock();
			printf("dequeue Total Times %d\n", end - begin);
		}
		moodycamel::ConsumerToken tokenCon(conMsgQueue);
		for (int j = 0; j < 5; j++)
		{
			clock_t begin = clock();
			for (int i = 0; i < TIMES_FAST; i++)
				conMsgQueue.try_dequeue(tokenCon, msg);
			clock_t end = clock();
			printf("dequeue Total Times %d\n", end - begin);
		}
	}*/
}


#endif