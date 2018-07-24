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

template<class T>
class CCLockfreeQueue : public BaseClass{
#define CCLockfreeQueuemalloc malloc
#define CCLockfreeQueuefree free
#define CCLockfreeMaxIndexLevel 16
#define CCLockfreeNextQueueTimes  2
#define CCLockfreeQueueData T
#define CCLockfreeLog printf
public:
    struct LockfreeQueueNode{
        CCLockfreeQueueData                     m_data;
        std::atomic<bool>                       m_bReadAlready;
    };
    
    class AllocateIndex : public BaseClass{
    public:
        AllocateIndex(uint64_t nI64IndexBegin, uint32_t nCount){
            m_nI64IndexBegin = nI64IndexBegin;
            m_nMaxCount = nCount;
            uint64_t nDis = UINT64_MAX - m_nI64IndexBegin;
            m_nI64IndexEnd = m_nI64IndexBegin + nCount;
            if(nDis < nCount){
                m_bNextBegin = true;
            }
            //
            m_pPool = (LockfreeQueueNode*)CCLockfreeQueuemalloc(sizeof(LockfreeQueueNode) * nCount);
            memset(m_pPool, 0, sizeof(LockfreeQueueNode) * nCount);
        }
        void reInit(uint64_t nI64IndexBegin){
            uint32_t nCount = m_nMaxCount;
            m_nI64IndexBegin = nI64IndexBegin;
            uint64_t nDis = UINT64_MAX - m_nI64IndexBegin;
            m_nI64IndexEnd = m_nI64IndexBegin + nCount;
            if(nDis < nCount){
                m_bNextBegin = true;
            }
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
        void setValue(int64_t nLocation, const CCLockfreeQueueData& value){
            LockfreeQueueNode& dataIndex = m_pPool[nLocation];
            dataIndex.m_data = value;
            dataIndex.m_bReadAlready.store(true, std::memory_order_release);
        }
        bool popValue(uint64_t nLocation, CCLockfreeQueueData& value){
            LockfreeQueueNode& dataIndex = m_pPool[nLocation];
            for(int i = 0;i < 100;i++){
                bool bRead = true;
                if(dataIndex.m_bReadAlready.compare_exchange_weak(bRead, false, std::memory_order_release, std::memory_order_acquire)){
                    //loop is uncorrect
                    value = dataIndex.m_data;
                    return true;
                }
            }
            assert(0);
            return false;
        }
        int64_t getPositionByIndex(uint64_t nI64Index){
            if(m_bNextBegin){
                if(nI64Index >= m_nI64IndexBegin)
                    return nI64Index - m_nI64IndexBegin;
                if(nI64Index < m_nI64IndexEnd)
                    return UINT64_MAX - m_nI64IndexBegin + nI64Index + 1;
            }
            else{
                if(nI64Index >= m_nI64IndexBegin){
                    if(nI64Index < m_nI64IndexEnd)
                        return nI64Index - m_nI64IndexBegin;
                }
            }
            return -1;
        }
        uint64_t getEndIndex(){
            return m_nI64IndexEnd;
        }
    protected:
        LockfreeQueueNode*          m_pPool;
        uint32_t		            m_nMaxCount;
        uint64_t                    m_nI64IndexBegin;
        uint64_t                    m_nI64IndexEnd;
        bool                        m_bNextBegin = false;
    };
public:
    CCLockfreeQueue(int nDefaultQueuePowerSize = 10){
        if(CCLockfreeMaxIndexLevel > 32 || CCLockfreeMaxIndexLevel < 4){
            printf("no support more 32 level or less 4 level\n");
            exit(0);
        }
        if(CCLockfreeNextQueueTimes > 8){
            printf("every time add times no more 8\n");
            exit(0);
        }
        m_nI64Read = 0;
        m_nI64Write = 0;
        m_nNextQueueSize = (uint32_t)pow(2, nDefaultQueuePowerSize);
        memset(m_pQueuePool, 0, CCLockfreeMaxIndexLevel * sizeof(AllocateIndex*));
        m_pQueuePoolRevert = nullptr;
        m_pQueuePool[0] = new AllocateIndex(0, m_nNextQueueSize);
        m_nNextQueueSize *= CCLockfreeNextQueueTimes;
    }
    virtual ~CCLockfreeQueue(){
        for(int i = 0; i < CCLockfreeMaxIndexLevel; i++){
            if(m_pQueuePool[i])
                delete m_pQueuePool[i];
        }
        if(m_pQueuePoolRevert)
            delete m_pQueuePoolRevert;
    }

    bool Push(const CCLockfreeQueueData& value){
        return PushByIndex(value, m_nI64Write.fetch_add(1, std::memory_order_relaxed));
    }

    bool Pop(CCLockfreeQueueData& value){
        uint64_t nWriteIndex = m_nI64Write.load(std::memory_order_relaxed);
        uint64_t nReadIndex = m_nI64Read.load(std::memory_order_relaxed);
        do{
            if(nReadIndex == nWriteIndex)
                return false;
        } while(!m_nI64Read.compare_exchange_weak(nReadIndex, nReadIndex + 1, std::memory_order_release, std::memory_order_relaxed));
        int64_t nCircleLocation = 0;
        AllocateIndex* pIndex = getReadCircleIndex(nReadIndex, nCircleLocation);
        if(pIndex == nullptr){
            assert(0);
            return false;
        }
        return pIndex->popValue(nCircleLocation, value);
    }
protected:
    bool PushByIndex(const CCLockfreeQueueData& value, uint64_t nPreWriteIndex){
        //last create index
        uint8_t nLastWriteIndex = m_nLastCreateIndex;

        int64_t nCircleLocation = 0;
        AllocateIndex* pAllocIndex = getWriteCircleIndex(nPreWriteIndex, nCircleLocation, nLastWriteIndex);
        if(pAllocIndex){
            pAllocIndex->setValue(nCircleLocation, value);
            return true;
        }
        //spin lock
        while(m_lockPool.exchange(1)){};
        if(nLastWriteIndex != m_nLastCreateIndex){
            m_lockPool.exchange(0);
            return PushByIndex(value, nPreWriteIndex);
        }
        uint32_t nLastCircle = nLastWriteIndex % CCLockfreeMaxIndexLevel;
        uint32_t nCreateIndex = (nLastWriteIndex + 1) % CCLockfreeMaxIndexLevel;

        assert(m_pQueuePool[nCreateIndex] == nullptr);
        assert(m_pQueuePool[nLastCircle] != nullptr);
        m_pQueuePool[nCreateIndex] = getAllocateIndex(m_pQueuePool[nLastCircle]->getEndIndex());
        m_nLastCreateIndex++;
        m_lockPool.exchange(0);
        return PushByIndex(value, nPreWriteIndex);
    }
    AllocateIndex* getAllocateIndex(uint64_t nEnd){
        if(m_pQueuePoolRevert){
            AllocateIndex* pRet = m_pQueuePoolRevert;
            m_pQueuePoolRevert = nullptr;
            pRet->reInit(nEnd);
            return pRet;
        }
        AllocateIndex* pRet = new AllocateIndex(nEnd, m_nNextQueueSize);
        CCLockfreeLog("expand_queue:%d\n", m_nNextQueueSize * sizeof(AllocateIndex));
        if(m_nNextQueueSize < UINT32_MAX / 16)
            m_nNextQueueSize *= CCLockfreeNextQueueTimes;
        return pRet;
    }

    //判断是否需要回收
    void reuseAllocateIndex(uint8_t nNowReadCircle){
        //判断上上个环是否可以回收
        uint8_t nIndex = (nNowReadCircle + CCLockfreeMaxIndexLevel - 2) % CCLockfreeMaxIndexLevel;
        AllocateIndex* pIndex = m_pQueuePool[nIndex];
        if(pIndex){
            //存在回收
            //spin lock
            AllocateIndex* pDelete = nullptr;
            while(m_lockPool.exchange(1)){};
            if(m_pQueuePool[nIndex] != pIndex){
                m_lockPool.exchange(0);
                return;
            }
            m_pQueuePool[nIndex] = nullptr;
            if(m_pQueuePoolRevert){
                if(m_pQueuePoolRevert->getMaxCount() > pIndex->getMaxCount()){
                    pDelete = pIndex;
                }
                else{
                    pDelete = m_pQueuePoolRevert;
                    m_pQueuePoolRevert = pIndex;
                }
            }
            else{
                m_pQueuePoolRevert = pIndex;
            }

            m_lockPool.exchange(0);
            if(pDelete)
                delete pDelete;
        }
    }
    //get position
    AllocateIndex* getWriteCircleIndex(uint64_t nI64Index, int64_t& nLocation, uint8_t nStartSearch){
        for(int i = 0; i < CCLockfreeMaxIndexLevel; i++){
            AllocateIndex* pIndex = m_pQueuePool[(nStartSearch + i) % CCLockfreeMaxIndexLevel];
            if(pIndex){
                nLocation = pIndex->getPositionByIndex(nI64Index);
                if(nLocation < 0){
                    continue;
                }
                return pIndex;
            }
            break;
        }
        return nullptr;
    }
    AllocateIndex* getReadCircleIndex(uint64_t nI64Index, int64_t& nLocation){
        uint8_t nStartSearch = m_nLastReadIndex;
        nLocation = m_pQueuePool[nStartSearch]->getPositionByIndex(nI64Index);
        if(nLocation >= 0){
            return m_pQueuePool[nStartSearch];
        }
        uint8_t nNextSearch = (nStartSearch + 1) % CCLockfreeMaxIndexLevel;
        nLocation = m_pQueuePool[nNextSearch]->getPositionByIndex(nI64Index);
        if(nLocation >= 0){
            m_nLastReadIndex = nNextSearch;
            //释放上上个环
            reuseAllocateIndex(m_nLastReadIndex);

            return m_pQueuePool[nNextSearch];
        }
        //most is not enter here, search last
        uint8_t nThirdSearch = (nStartSearch + CCLockfreeMaxIndexLevel - 1) % CCLockfreeMaxIndexLevel;
        nLocation = m_pQueuePool[nThirdSearch]->getPositionByIndex(nI64Index);
        if(nLocation >= 0){
            return m_pQueuePool[nThirdSearch];
        }
        assert(0);
        return nullptr;
    }
protected:
    std::atomic<uint64_t>   m_nI64Read;
    std::atomic<uint64_t>   m_nI64Write;
    std::atomic<char>		m_lockPool;
    AllocateIndex*			m_pQueuePool[CCLockfreeMaxIndexLevel];        //
    AllocateIndex*          m_pQueuePoolRevert;
    uint32_t				m_nNextQueueSize = 0;
    uint8_t                 m_nLastCreateIndex = 0;
    uint8_t                 m_nLastReadIndex = 0;
};
