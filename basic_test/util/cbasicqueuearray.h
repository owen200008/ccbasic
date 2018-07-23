#pragma once

#include <stdlib.h>
#include <atomic>
#include <assert.h>

//must be pow(2) BASICQUEUE_MAX_ALLOCTIMES=max allocate times BASICQUEUE_ALLOCMULTYTIMES=every time allocate more memory size BASICQUEUE_DELETEQUEUESIZE=if allocate > 4096*BASICQUEUE_ALLOCMULTYTIME*BASICQUEUE_ALLOCMULTYTIMES delete less memory allocate
//default 1.allocate 16  2.allocate 64 3.allocate 256 4.allocate 1024 бн 16 times
template<class T, uint32_t BASICQUEUE_MAX_ALLOCTIMES = 16, uint32_t BASICQUEUE_ALLOCMULTYTIMES = 4, uint32_t BASICQUEUE_DELETEQUEUESIZE = 16384>
class CBasicQueueArray : public basiclib::CBasicObject{
public:
    struct ArrayNode{
        T                                        m_data;
        std::atomic<bool>                        m_bReadAlready;
    };

    class AllocateIndexData : public basiclib::CBasicObject{
    public:
        inline uint32_t GetDataIndex(uint32_t uValue){
            return uValue % m_nMaxCount;
        }
        AllocateIndexData(int nCount){
            m_nMaxCount = nCount;
            m_nNoRead = 0;
            m_nRead = 0;
            m_nPreWrite = 0;
            m_pPool = (ArrayNode*)basiclib::BasicAllocate(sizeof(ArrayNode) * nCount);
            memset(m_pPool, 0, sizeof(ArrayNode) * nCount);
        }
        virtual ~AllocateIndexData(){
            if(m_pPool){
                basiclib::BasicDeallocate(m_pPool);
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
            /*uint32_t nReadIndex = m_nRead.fetch_add(1, std::memory_order_relaxed);
            nReadIndex -= m_nNoRead.load(std::memory_order_relaxed);
            ArrayNode& node = m_pPool[GetDataIndex(nReadIndex)];
            bool bTrue = true;
            if(!node.m_bReadAlready.compare_exchange_weak(bTrue, false, std::memory_order_release, std::memory_order_relaxed)){
                m_nNoRead.fetch_add(1, std::memory_order_relaxed);
                return false;
            }
            value = pNode->m_data;
            pNode->m_bReadAlready.store(false, std::memory_order_relaxed);
            return true;
            */



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
        std::atomic<uint32_t>    m_nNoRead;
        std::atomic<uint32_t>    m_nPreWrite;
        uint32_t                m_nMaxCount;
        ArrayNode*                m_pPool;
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
        m_pQueuePoolRevert = nullptr;
    }
    virtual ~CBasicQueueArray(){
        for(int i = 0; i < BASICQUEUE_MAX_ALLOCTIMES; i++){
            if(m_pMaxAllocTimes[i])
                delete m_pMaxAllocTimes[i];
        }
    }
    bool Push(const T& value, int nDeep = 0){
        if(nDeep >= BASICQUEUE_MAX_ALLOCTIMES){
            ASSERT(0);
            return false;
        }
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
            m_pMaxAllocTimes[nWriteIndex] = new AllocateIndexData(m_nNextQueueSize);
            m_nNextQueueSize *= BASICQUEUE_ALLOCMULTYTIMES;
            TRACE("expand_queue:%d\n", m_nNextQueueSize / BASICQUEUE_ALLOCMULTYTIMES * sizeof(ArrayNode));
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
            m_lock.exchange(0, std::memory_order_relaxed);
            if(pDelete){
                delete pDelete;
            }
        }
        return Pop(value);
    }
    bool IsEmpty(){
        uint32_t nReadIndex = GetQueueArrayIndex(m_cReadIndex.load(std::memory_order_relaxed));
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
        return false;
    }
protected:
    uint32_t                                                    m_nNextQueueSize;

    std::atomic<uint32_t>                                        m_cReadIndex;
    std::atomic<uint32_t>                                        m_cWriteIndex;

    std::atomic<char>                                            m_lock;
    AllocateIndexData*                                           m_pMaxAllocTimes[BASICQUEUE_MAX_ALLOCTIMES];
    AllocateIndexData*                                           m_pQueuePoolRevert;
};
