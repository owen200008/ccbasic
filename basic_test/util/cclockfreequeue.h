#pragma once

#include <stdlib.h>
#include <atomic>
#include <assert.h>

struct CCLockfreeQueueMgrFunc{
    //! (2的指数幂)最大分配次数, 这个值越大对象占用内存越大 sizeof（指针） * BASICQUEUE_MAX_ALLOCTIMES
    static const uint32_t CCLOCKFREEQYEYE_MAX_ALLOCTIMES = 16;
    //! (2的指数幂)每次分配增大的倍数
    static const uint32_t CCLOCKFREEQYEYE_ALLOCMULTYTIMES = 2;


    //! 每个block的size
    static const uint32_t CCLockfreeQueue_Block_Size = 64;

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

template<class Traits = CCLockfreeQueueMgrFunc>
class CCLockfreeQueueObject{
public:
    CCLockfreeQueueObject(){
    }
    virtual ~CCLockfreeQueueObject(){
    }

    // Diagnostic allocations
    void* operator new(size_t nSize){
        return Traits::malloc(nSize);
    }
    void operator delete(void* p){
        Traits::free(p);
    }
};


template<class T, class Traits = BasicQueueArrayMgrFunc, class ObjectBaseClass = CCLockfreeQueueObject<Traits>>
class CCLockfreeQueue : public ObjectBaseClass{
public:
    struct ArrayNode{
        T                                        m_data;
        std::atomic<bool>                        m_bReadAlready;
    };
    struct Block {
    public:
        void reInit(uint32_t nIndex, Block* pPre){
            m_nIndex = nIndex;
            pPre->m_pNextBlock = this;
            this->m_pPreBlock = pPre;
        }
    protected:
        uint32_t        m_nIndex;
        ArrayNode       m_pool[Traits::CCLockfreeQueue_Block_Size];
        Block*          m_pNextBlock;
        Block*          m_pPreBlock;
    };
    struct MemoryAlloc{
        MemoryAlloc*    m_pPre;
    };
    class AllocateIndexData : public ObjectBaseClass{
    public:
        inline uint32_t GetDataIndex(uint32_t uValue){
            return uValue % m_nMaxCount;
        }
        AllocateIndexData(int nCount){
            m_nMaxCount = nCount;
            m_nRead = 0;
            m_nPreWrite = 0;
            m_pPool = (ArrayNode*)Traits::malloc(sizeof(ArrayNode) * nCount);
            memset(m_pPool, 0, sizeof(ArrayNode) * nCount);
        }
        virtual ~AllocateIndexData(){
            if(m_pPool){
                Traits::free(m_pPool);
            }
        }
        void Reset(){
            m_nPreWrite = 0;
            m_nRead = 0;
        }
        bool Push(const T& value){
            uint32_t nWriteIndex = m_nPreWrite.fetch_add(1, std::memory_order_relaxed);
            ArrayNode& node = m_pPool[GetDataIndex(nWriteIndex)];
            if(node.m_bReadAlready.load(std::memory_order_relaxed)){
                //nWriteIndex overflow need reset
                //full
                return false;
            }
            node.m_data = value;
            node.m_bReadAlready.store(true, std::memory_order_release);
            return true;
        }
        bool Pop(T& value){
            uint32_t nReadIndex = m_nRead.load(std::memory_order_relaxed);
            ArrayNode* pNode = nullptr;
            do{
                pNode = &(m_pPool[GetDataIndex(nReadIndex)]);
                if(!pNode->m_bReadAlready.load(std::memory_order_acquire)){
                    //empty
                    if(m_nRead.compare_exchange_weak(nReadIndex, nReadIndex, std::memory_order_release, std::memory_order_relaxed)){
                        return false;
                    }
                    //continue must be true, if no continue to compare_exchange_weak pNode is error.
                    continue;
                }
                if(m_nRead.compare_exchange_weak(nReadIndex, nReadIndex + 1, std::memory_order_release, std::memory_order_relaxed))
                    break;
            } while(true);
            value = pNode->m_data;
            pNode->m_bReadAlready.store(false, std::memory_order_relaxed);
            return true;
        }
        bool IsEmpty(){
            uint32_t nReadIndex = m_nRead.load(std::memory_order_relaxed);
            do{
                ArrayNode * pNode = &(m_pPool[GetDataIndex(nReadIndex)]);
                if(!pNode->m_bReadAlready.load(std::memory_order_acquire)){
                    //empty
                    if(m_nRead.compare_exchange_weak(nReadIndex, nReadIndex, std::memory_order_release, std::memory_order_relaxed)){
                        return true;
                    }
                    continue;
                }
                break;
            } while(true);
            return false;
        }
        uint32_t GetAllocCount(){ return m_nMaxCount; }
    public:
        std::atomic<uint32_t>    m_nRead;
        std::atomic<uint32_t>    m_nPreWrite;
        uint32_t                m_nMaxCount;
        ArrayNode*                m_pPool;
    };
protected:
    //add to memoryalloc
    inline Block* create_block(size_t count){
        MemoryAlloc* pAlloc = static_cast<MemoryAlloc*>((Traits::malloc)(sizeof(Block) * count + sizeof(MemoryAlloc)));
        pAlloc->m_pPre = m_pMemoryAlloc;
        m_pMemoryAlloc = pAlloc;
        auto p = static_cast<U*>(pAlloc + 1);
        if(p == nullptr)
            return nullptr;
        for(size_t i = 0; i != count; ++i){
            new (p + i) U();
        }
        return p;
    }
public:
    inline uint32_t GetQueueArrayIndex(uint32_t nIndex){
        return nIndex % Traits::BASICQUEUE_MAX_ALLOCTIMES;
    }
    CCLockfreeQueue(uint16_t nDefaultBlockSize = 4){
        if(nDefaultBlockSize < 4)
            nDefaultBlockSize = 4;
        m_nPreReadIndex = 0;
        m_nReadIndex = 0;
        m_nWriteIndex = 0;
        m_nNextBlockSize = nDefaultBlockSize;
        m_lock = 0;
        Block* pBlock = create_block(m_nNextBlockSize);
        m_pHead = pBlock;
        m_pTail = pBlock;
        

        m_pMaxAllocTimes[0] = new AllocateIndexData(m_nNextBlockSize);
        m_nNextBlockSize *= Traits::BASICQUEUE_ALLOCMULTYTIMES;
        m_pQueuePoolRevert = nullptr;
    }
    virtual ~CCLockfreeQueue(){
        for(int i = 0; i < Traits::BASICQUEUE_MAX_ALLOCTIMES; i++){
            if(m_pMaxAllocTimes[i])
                delete m_pMaxAllocTimes[i];
        }
    }
    bool Push(const T& value){
        uint32_t nWriteIndex = m_nWriteIndex.fetch_add(1, std::memory_order_relaxed);
        //get block num and index
        uint32_t nBlockIndex = nWriteIndex / Traits::CCLockfreeQueue_Block_Size; 
        uint32_t nBlockPos = nWriteIndex % Traits::CCLockfreeQueue_Block_Size;
        //get real block index



        uint32_t nGetWriteIndex = m_cWriteIndex.load(std::memory_order_relaxed);
        uint32_t nWriteIndex = GetQueueArrayIndex(nGetWriteIndex);

        AllocateIndexData* pAllocData = m_pMaxAllocTimes[nWriteIndex];
        if(pAllocData){
            if(pAllocData->Push(value))
                return true;
            m_cWriteIndex.compare_exchange_weak(nGetWriteIndex, nGetWriteIndex + 1, std::memory_order_release, std::memory_order_relaxed);
            return Push(value, ++nDeep);
        }
        //spin lock
        while(m_lock.exchange(1, std::memory_order_relaxed)){};
        if(m_pMaxAllocTimes[nWriteIndex]){
            m_lock.exchange(0, std::memory_order_relaxed);
            return Push(value, nDeep);
        }
        //first find cache
        if(m_pQueuePoolRevert){
            m_pMaxAllocTimes[nWriteIndex] = m_pQueuePoolRevert;
            m_pMaxAllocTimes[nWriteIndex]->Reset();
            m_pQueuePoolRevert = nullptr;
        }
        else{
            m_pMaxAllocTimes[nWriteIndex] = new AllocateIndexData(m_nNextBlockSize);
            m_nNextBlockSize *= Traits::BASICQUEUE_ALLOCMULTYTIMES;
            Traits::Trace("expand_queue:%d\n", m_nNextBlockSize / Traits::BASICQUEUE_ALLOCMULTYTIMES * sizeof(ArrayNode));
        }
        m_lock.exchange(0, std::memory_order_relaxed);
        return Push(value, nDeep);
    }
    bool Pop(T& value){
        uint32_t nGetReadIndex = m_cReadIndex.load(std::memory_order_relaxed);
        uint32_t nReadIndex = GetQueueArrayIndex(nGetReadIndex);
        AllocateIndexData* pAllocData = m_pMaxAllocTimes[nReadIndex];
        if(!pAllocData){
            //speical case.
            return false;
        }
        if(pAllocData->Pop(value)){
            return true;
        }

        //empty read write same circle
        uint32_t nWriteIndex = GetQueueArrayIndex(m_cWriteIndex.load(std::memory_order_relaxed));
        if(nWriteIndex == nReadIndex){
            //empty
            return false;
        }

        //next circle
        if(m_cReadIndex.compare_exchange_weak(nGetReadIndex, nGetReadIndex + 1, std::memory_order_release, std::memory_order_relaxed)){
            AllocateIndexData* pDelete = nullptr;
            while(m_lock.exchange(1, std::memory_order_relaxed)){};
            if(m_pQueuePoolRevert){
                if(m_pQueuePoolRevert->GetAllocCount() < pAllocData->GetAllocCount()){
                    pDelete = m_pQueuePoolRevert;
                    m_pQueuePoolRevert = pAllocData;
                }
                else{
                    pDelete = pAllocData;
                }
            }
            else{
                m_pQueuePoolRevert = pAllocData;
            }
            m_pMaxAllocTimes[nReadIndex] = nullptr;
            m_lock.exchange(0, std::memory_order_relaxed);
            if(pDelete){
                delete pDelete;
            }
        }
        return Pop(value);
    }
protected:
    uint32_t                                                    m_nNextBlockSize;

    std::atomic<uint32_t>                                       m_nPreReadIndex;
    std::atomic<uint32_t>                                       m_nReadIndex;
    std::atomic<uint32_t>                                       m_nWriteIndex;

    std::atomic<char>                                           m_lock;

    std::atomic<Block*>                                         m_pHead;
    std::atomic<Block*>                                         m_pTail;

    MemoryAlloc*                                                m_pMemoryAlloc;

    AllocateIndexData*                                           m_pMaxAllocTimes[Traits::BASICQUEUE_MAX_ALLOCTIMES];
    AllocateIndexData*                                           m_pQueuePoolRevert;
};
