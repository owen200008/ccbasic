#pragma once

#include <stdlib.h>
#include <atomic>
#include <assert.h>

struct CCQueueMgrFunc{
    static const uint32_t BASICQUEUE_MAX_ALLOCTIMES = 16;

    static const uint32_t BASICQUEUE_ALLOCMULTYTIMES = 4;

    static const uint32_t BASICQUEUE_DELETEQUEUESIZE = 16384;

    static const uint32_t BASICQUEUE_DELETEQUEUECHECKNEXTSIZE = BASICQUEUE_DELETEQUEUESIZE * BASICQUEUE_ALLOCMULTYTIMES * BASICQUEUE_ALLOCMULTYTIMES * BASICQUEUE_ALLOCMULTYTIMES;
#if defined(malloc) || defined(free)
    static inline void* malloc(size_t size){ return ::malloc(size); }
    static inline void free(void* ptr){ return ::free(ptr); }
#else
    static inline void* malloc(size_t size){ return std::malloc(size); }
    static inline void free(void* ptr){ return std::free(ptr); }
#endif
    template<class... _Types>
    static inline void Trace(const char* pData, _Types&&... _Args){
#ifdef _DEBUG
        printf(pData, std::forward<_Types>(_Args)...);
#endif
    }
};

template<class Traits = CCQueueMgrFunc>
class CCQueueObject{
public:
    CCQueueObject(){
    }
    virtual ~CCQueueObject(){
    }

    // Diagnostic allocations
    void* operator new(size_t nSize){
        return Traits::malloc(nSize);
    }
    void operator delete(void* p){
        Traits::free(p);
    }
};

//template<class Traits = CCQueueMgrFunc, class BaseClass = CCQueueObject<Traits>>
#define BaseClass CCQueueObject<CCQueueMgrFunc>

class CCLockfreeQueue : public BaseClass{
#define CCLockfreeQueuemalloc malloc
#define CCLockfreeQueuefree free
#define CCLockfreeMaxIndexLevel 8
#define CCLockfreeNextQueueTimes  2
#define CCLockfreeQueueData uint32_t
#define CCLockfreeLog printf
public:
    struct LockfreeQueueNode{
        T                                       m_data;
        std::atomic<bool>                       m_bReadAlready = false;
    };
    
    class AllocateIndex : public BaseClass{
    public:
        AllocateIndex(uint64_t nIndexBegin, int nCount){
            m_nIndexBegin = nIndexBegin;
            m_nMaxCount = nCount;
            m_nIndexEnd = m_nIndexBegin + m_nMaxCount;
            //
            m_pPool = (LockfreeQueueNode*)CCLockfreeQueuemalloc(sizeof(LockfreeQueueNode) * nCount);
            memset(m_pPool, 0, sizeof(LockfreeQueueNode) * nCount);
        }
        virtual ~AllocateIndex(){
            if(m_pPool){
                CCLockfreeQueuefree(m_pPool);
            }
        }
        uint32_t getMaxCount(){
            return m_nMaxCount;
        }
        uint32_t getIndexBegin(){
            return m_nIndexBegin;
        }
        void setValue(uint32_t nLocation, const T& value){
            AllocateIndex& dataIndex = m_pPool[nPoolIndex];
            dataIndex.m_data = value;
            dataIndex.m_bRead = false;
            dataIndex.m_bReadAlready.store(true, std::memory_order_release);
        }
        bool popValue(uint32_t nLocation, const T& value){
            AllocateIndex& dataIndex = m_pPool[nPoolIndex];
            for(int i = 0;i < 100;i++){
                bool bRead = true;
                if(dataIndex.m_bReadAlready.compare_exchange_weak(bRead, false, std::memory_order_release, std::memory_order_acquire)){
                    //loop is uncorrect
                    value = dataIndex.value;
                    return true;
                }
            }
            return false;
        }
        
        //is inside
        int compareIndexValue(uint32_t nIndex){
            if(nIndex >= m_nIndexBegin){
                if(nIndex < m_nIndexEnd)
                    return 0;
                return 1;
            }
            return -1;
        }
    protected:
        LockfreeQueueNode*          m_pPool;
        uint32_t		            m_nMaxCount;
        uint32_t                    m_nIndexBegin;
        uint32_t                    m_nIndexEnd;
        bool                        m_bNextBegin = false;
        uint32_t                    m_nIndexBeginSecond = 0;
        uint32_t                    m_nIndexEndSecond = 0;
    };
public:
    CCLockfreeQueue(int nDefaultQueuePowerSize = 10){
        if(CCLockfreeMaxIndexLevel > 32){
            exit(0);
        }
        m_nNextQueueSize = (uint32_t)pow(2, nDefaultQueuePowerSize);
        memset(m_pQueuePool, 0, CCLockfreeMaxIndexLevel * sizeof(AllocateIndex*));
        m_pQueuePool[0] = new AllocateIndex(0, m_nNextQueueSize);
        m_nNextQueueSize *= CCLockfreeNextQueueTimes;
    }
    virtual ~CCLockfreeQueue(){
        for(int i = 0; i < CCLockfreeMaxIndexLevel; i++){
            if(m_pQueuePool[i])
                delete m_pQueuePool[i];
        }
    }
    //get position
    bool getWriteCircleIndex(uint32_t nIndex, uint32_t& nWriteIndex, uint32_t& nLocation){
        int nLastWriteIndex = m_nLastCreateIndex;
        for(int i = 0; i < CCLockfreeMaxIndexLevel; i++){
            AllocateIndex* pIndex = m_pQueuePool[(nLastWriteIndex + CCLockfreeMaxIndexLevel) % CCLockfreeMaxIndexLevel];
            if(pIndex){
                switch(m_pQueuePool[i]->compareIndexValue(nIndex)){
                case 0:
                    nLocation = nIndex - m_pQueuePool[i]->getIndexBegin();
                    nWriteIndex = i;
                    return true;
                case 1:
                    nLastWriteIndex++;
                    continue;
                default:
                    nLastWriteIndex--;
                    continue;
                }
            }
            else{
                nWriteIndex = i;
                return false;
            }
        }
        return false;
    }

    bool Push(const CCLockfreeQueueData& value, int nDeep = 0){
        if(nDeep >= CCLockfreeMaxIndexLevel){
            CCLockfreeLog("Push Too Deep %d", nDeep);
            assert(0);
            return false;
        }

        uint32_t nPreWriteIndex = m_nWrite.fetch_add(1);
        uint32_t nWriteIndex = 0;
        uint32_t nLocation = 0;
        if(getWriteCircleIndex(nPreWriteIndex, nWriteIndex, nLocation)){
            m_pQueuePool[nWriteIndex]->setValue(nLocation, nSetLocationIndex);
            return true;
        }
        //spin lock
        while(m_lockPool.exchange(1)){};
        if(m_pQueuePool[nWriteIndex]){
            m_lockPool.exchange(0);
            return Push(value, nDeep);
        }
        m_pQueuePool[nWriteIndex] = new AllocateIndex(, m_nNextQueueSize);
        m_nNextQueueSize *= Traits::BASICQUEUE_ALLOCMULTYTIMES;
        m_lock.exchange(0, std::memory_order_relaxed);
        Traits::Trace("expand_queue:%d\n", m_nNextQueueSize / Traits::BASICQUEUE_ALLOCMULTYTIMES * sizeof(ArrayNode));
        return Push(value, nDeep);
    }

    void Expand_Pool(){
        uint32_t nPoolSize = m_nCurrentPoolSize.load(std::memory_order_relaxed);
        //spin lock
        while(m_lockPool.exchange(1)){};
        if(nPoolSize != m_nCurrentPoolSize.load(std::memory_order_acquire)){
            m_lockPool.exchange(0);
            return;
        }
        if(m_pQueuePoolRevert)
            delete[]m_pQueuePoolRevert;
        m_pQueuePoolRevert = new uint32_t[nSize];
        memcpy(m_pQueuePoolRevert, m_pQueuePool, sizeof(uint32_t) * nPoolSize);
        swap(m_pQueuePool, m_pQueuePoolRevert);
        m_nCurrentPoolSize.store(nSize);
        m_lock.exchange(0);
    }
protected:
    std::atomic<uint64_t>   m_nRead = 0;
    std::atomic<uint64_t>   m_nWrite = 0;
    std::atomic<char>		m_lockPool;
    AllocateIndex*			m_pQueuePool[CCLockfreeMaxIndexLevel];        //
    uint32_t				m_nNextQueueSize = 0;
    uint32_t                m_nLastCreateIndex = 0;
};
