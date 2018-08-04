#pragma once

#include <stdlib.h>
#include <atomic>
#include <assert.h>

struct CCLockfreeQueueFunc{
    //! (2的指数幂)最大分配次数, 这个值越大对象占用内存越大 sizeof（指针） * BASICQUEUE_MAX_ALLOCTIMES
    static const uint32_t BASICQUEUE_MAX_ALLOCTIMES = 16;
    //! (2的指数幂)每次分配增大的倍数
    static const uint32_t BASICQUEUE_ALLOCMULTYTIMES = 4;

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

template<class Traits = CCLockfreeQueueFunc>
class CCLockfreeObject{
public:
    CCLockfreeObject(){
    }
    virtual ~CCLockfreeObject(){
    }

    // Diagnostic allocations
    void* operator new(size_t nSize){
        return Traits::malloc(nSize);
    }
    void operator delete(void* p){
        Traits::free(p);
    }
};


template<class T, class Traits = CCLockfreeQueueFunc, class ObjectBaseClass = CCLockfreeObject<Traits>>
class CCLockfreeQueue : public ObjectBaseClass{
public:
    struct ArrayNode{
        T                                        m_data;
        std::atomic<uint64_t>                    m_i64SignPosition;
    };
#define CCLockfreeQueueArrayNodeSign_Mask       0xffffffff00000000
#define CCLockfreeQueueArrayNodeSign_OperAdd    0x0000000100000000

    class AllocateIndexData : public ObjectBaseClass{
    public:
        inline uint32_t GetDataIndex(uint32_t uValue){
            return uValue % m_nMaxCount;
        }
        AllocateIndexData(int nCount){
            m_nMaxCount = nCount;
            m_pPool = (ArrayNode*)Traits::malloc(sizeof(ArrayNode) * nCount);
            memset(m_pPool, 0, sizeof(ArrayNode) * nCount);
        }
        virtual ~AllocateIndexData(){
            if(m_pPool){
                Traits::free(m_pPool);
            }
        }
        bool PushByPosition(const T& value, uint32_t& nWritePosition){
            ArrayNode& node = m_pPool[GetDataIndex(nWritePosition)];
            uint64_t i64SignLastWritePos = node.m_i64SignPosition.fetch_add(CCLockfreeQueueArrayNodeSign_OperAdd, std::memory_order_relaxed);
            if((i64SignLastWritePos & CCLockfreeQueueArrayNodeSign_Mask) != CCLockfreeQueueArrayNodeSign_OperAdd){
                uint32_t nLastWritePos = i64SignLastWritePos & 0xffffffff;
                if(nLastWritePos == 0){
                    //wait to write finish
                    return PushByPosition(value, nWritePosition);
                }
                if(nWritePosition < nLastWritePos){
                    uint64_t i64SetPosition = CCLockfreeQueueArrayNodeSign_OperAdd;
                    i64SetPosition += nWritePosition;
                    if(!node.m_i64SignPosition.compare_exchange_strong(i64SignLastWritePos, i64SetPosition, std::memory_order_acquire, std::memory_order_relaxed)){
                        return PushByPosition(value, nWritePosition);
                    }
                    nWritePosition = nLastWritePos;
                }
                //nWriteIndex overflow need reset
                //full
                return false;
            }
            node.m_data = value;
            node.m_i64SignPosition.fetch_add(nWritePosition, std::memory_order_release);
            return true;
        }
        bool PopByPosition(T& value, uint32_t nReadPosition){
            ArrayNode& node = m_pPool[GetDataIndex(nReadPosition)];
            uint64_t i64SignLastWritePos = node.m_i64SignPosition.load(std::memory_order_acquire);
            uint32_t nLastWritePosition = i64SignLastWritePos & 0xffffffff;
            if(nLastWritePosition == nReadPosition){
                value = node.m_data;
                node.m_i64SignPosition.store(0, std::memory_order_relaxed);
                return true;
            }
            else if(nReadPosition < nLastWritePosition){
                return PopByPosition(value, nReadPosition);
            }
            else if(nLastWritePosition == 0){
                if(){
                    
                }
            }
            //nReadPosition > nLastWritePosition
            return false;
        }
        uint32_t GetAllocCount(){ return m_nMaxCount; }
    public:
        uint32_t                    m_nMaxCount;
        ArrayNode*                  m_pPool;
    };
public:
    inline uint8_t GetQueueArrayIndex(uint8_t nIndex){
        return nIndex % Traits::BASICQUEUE_MAX_ALLOCTIMES;
    }
    CCLockfreeQueue(int nDefaultQueuePowerSize = 4){
        if(Traits::BASICQUEUE_MAX_ALLOCTIMES < 4){
            printf("Traits::BASICQUEUE_MAX_ALLOCTIMES mustbe more than 4");
            exit(0);
        }
        else if(UINT8_MAX % Traits::BASICQUEUE_MAX_ALLOCTIMES != 0){
            printf("Traits::BASICQUEUE_MAX_ALLOCTIMES mustbe more than pow of 2");
            exit(0);
        }
        m_cReadIndex = 0;
        m_cWriteIndex = 0;
        //must be begin with 1, 0 index is empty sign
        m_nPreReadPosition = 1;
        m_nReadPosition = 1;
        m_nPreWritePosition = 1;
        m_nWritePosition = 1;
        m_lock = 0;
        m_nNextQueueSize = (uint32_t)pow(2, nDefaultQueuePowerSize);

        memset(m_pMaxAllocTimes, 0, Traits::BASICQUEUE_MAX_ALLOCTIMES * sizeof(AllocateIndexData*));
        m_pMaxAllocTimes[0] = new AllocateIndexData(m_nNextQueueSize);
        m_nNextQueueSize *= Traits::BASICQUEUE_ALLOCMULTYTIMES;
        m_pQueuePoolRevert = nullptr;
    }
    virtual ~CCLockfreeQueue(){
        for(int i = 0; i < Traits::BASICQUEUE_MAX_ALLOCTIMES; i++){
            if(m_pMaxAllocTimes[i])
                delete m_pMaxAllocTimes[i];
        }
    }
    bool PushByWriteIndex(const T& value, uint8_t cGetWriteIndex, uint32_t nWritePosition){
        uint8_t cWriteIndex = GetQueueArrayIndex(cGetWriteIndex);
        AllocateIndexData* pAllocData = m_pMaxAllocTimes[cWriteIndex];
        if(pAllocData){
            if(pAllocData->PushByPosition(value, nWritePosition)){
                m_nWritePosition.fetch_add(1, std::memory_order_relaxed);
                return true;
            }
            return PushByWriteIndex(value, cGetWriteIndex + 1, nWritePosition);
        }
        //spin lock
        while(m_lock.exchange(1, std::memory_order_acq_rel)){};
        if(m_pMaxAllocTimes[cWriteIndex]){
            m_lock.exchange(0, std::memory_order_acq_rel);
            return PushByWriteIndex(value, cGetWriteIndex, nWritePosition);
        }
        //first find cache
        if(m_pQueuePoolRevert){
            m_pMaxAllocTimes[cWriteIndex] = m_pQueuePoolRevert;
            m_pQueuePoolRevert = nullptr;
        }
        else{
            m_pMaxAllocTimes[cWriteIndex] = new AllocateIndexData(m_nNextQueueSize);
            m_nNextQueueSize *= Traits::BASICQUEUE_ALLOCMULTYTIMES;
            Traits::Trace("expand_queue:%d\n", m_nNextQueueSize / Traits::BASICQUEUE_ALLOCMULTYTIMES * sizeof(ArrayNode));
        }
        m_lock.exchange(0, std::memory_order_acq_rel);
        return PushByWriteIndex(value, cGetWriteIndex, nWritePosition);
    }

    bool Push(const T& value){
        return PushByWriteIndex(value, m_cWriteIndex.load(std::memory_order_relaxed), m_nPreWritePosition.fetch_add(1, std::memory_order_relaxed));
    }
    bool Pop(T& value){
        uint8_t cGetReadIndex = m_cReadIndex.load(std::memory_order_relaxed);
        uint32_t nPreReadPosition = m_nPreReadPosition.fetch_add(1, std::memory_order_relaxed);
        uint32_t nWritePosition = m_nWritePosition.load(std::memory_order_relaxed);
        if(nPreReadPosition == nWritePosition){
            //empty
            return false;
        }
        uint32_t nReadPosition = m_nReadPosition.fetch_add(1, std::memory_order_relaxed);
        uint8_t cReadIndex = GetQueueArrayIndex(cGetReadIndex);
        AllocateIndexData* pAllocData = m_pMaxAllocTimes[cReadIndex];
        if(!pAllocData){
            //speical case.
            return false;
        }
        if(pAllocData->PopByPosition(value, nReadPosition)){
            return true;
        }
        //if exist next index,
        

        //empty read write same circle
        uint32_t nWriteIndex = GetQueueArrayIndex(m_cWriteIndex.load(std::memory_order_relaxed));
        if(nWriteIndex == nReadIndex){
            //empty
            return false;
        }

        //next circle
        if(m_cReadIndex.compare_exchange_weak(nGetReadIndex, nGetReadIndex + 1, std::memory_order_release, std::memory_order_relaxed)){
            AllocateIndexData* pDelete = nullptr;
            while(m_lock.exchange(1, std::memory_order_acq_rel)){};
            if(m_pQueuePoolRevert){
                pDelete = m_pQueuePoolRevert;
            }
            m_pQueuePoolRevert = pAllocData;
            m_pMaxAllocTimes[nReadIndex] = nullptr;
            m_lock.exchange(0, std::memory_order_acq_rel);
            if(pDelete){
                delete pDelete;
            }
        }
        return Pop(value);
    }
    bool IsEmpty(){
        /*uint32_t nReadIndex = GetQueueArrayIndex(m_cReadIndex.load(std::memory_order_relaxed));
        AllocateIndexData* pAllocData = m_pMaxAllocTimes[nReadIndex];
        if(!pAllocData){
            //speical case.
            return true;
        }
        if(!pAllocData->IsEmpty())
            return false;

        //empty read write same circle
        uint32_t nWriteIndex = GetQueueArrayIndex(m_cWriteIndex.load(std::memory_order_relaxed));
        if(nWriteIndex == nReadIndex){
            //empty
            return true;
        }
        return false;*/
    }
protected:
    uint32_t                                                    m_nNextQueueSize;

    std::atomic<uint8_t>                                        m_cReadIndex;
    std::atomic<uint8_t>                                        m_cWriteIndex;

    std::atomic<uint32_t>                                       m_nPreReadPosition;
    std::atomic<uint32_t>                                       m_nReadPosition;
    std::atomic<uint32_t>                                       m_nPreWritePosition;
    std::atomic<uint32_t>                                       m_nWritePosition;


    std::atomic<char>                                           m_lock;
    AllocateIndexData*                                          m_pMaxAllocTimes[Traits::BASICQUEUE_MAX_ALLOCTIMES];
    AllocateIndexData*                                          m_pQueuePoolRevert;
};
