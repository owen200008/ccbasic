#pragma once

#include <stdlib.h>
#include <atomic>
#include <assert.h>
#include <basic.h>

class atomic_backoff{
    //! Time delay, in units of "pause" instructions.
    /** Should be equal to approximately the number of "pause" instructions
    that take the same time as an context switch. Must be a power of two.*/
    static const int32_t LOOPS_BEFORE_YIELD = 16;
    int32_t count;
public:
    // In many cases, an object of this type is initialized eagerly on hot path,
    // as in for(atomic_backoff b; ; b.pause()) { /*loop body*/ }
    // For this reason, the construction cost must be very small!
    atomic_backoff() : count(1){}
    // This constructor pauses immediately; do not use on hot paths!
    atomic_backoff(bool) : count(1){ pause(); }

    static inline void pause(uintptr_t delay){
        for(; delay>0; --delay)
            _mm_pause();
    }
    //! Pause for a while.
    void pause(){
        if(count <= LOOPS_BEFORE_YIELD){
            pause(count);
            // Pause twice as long the next time.
            count *= 2;
        }
        else{
            // Pause is so long that we might as well yield CPU to scheduler.
#ifdef _WIN32
            SwitchToThread();
#else
            std::this_thread::yield();
#endif
        }
    }

    //! Pause for a few times and return false if saturated.
    bool bounded_pause(){
        pause(count);
        if(count<LOOPS_BEFORE_YIELD){
            // Pause twice as long the next time.
            count *= 2;
            return true;
        }
        else{
            return false;
        }
    }

    void reset(){
        count = 1;
    }
};


struct CCLockfreeQueueFunc{
    //! 避免冲突队列，不同队列减少冲突, 2的指数
    static const uint8_t ThreadWriteIndexModeIndex = 8;
    //! block size, , 2的指数
    static const uint16_t BlockPerSize = 32;
    //! 下一次分配的增大的倍数
    static const uint32_t BlockCountAddTimesNextTime = 2;

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
    class Block : public ObjectBaseClass, basiclib::CCLockfreeStackNode{
    public:
        struct StoreData{
            T                       m_pData;
            std::atomic<bool>       m_bWrite;
            StoreData() : m_bWrite(false){
            }
        };
    public:
        void Init(uint32_t nBeginIndex = 0){
            m_nBeginIndex = nBeginIndex;
        }
        inline void PushLocation(const T& value, uint32_t nPreWriteLocation){
            StoreData& node = m_pPool[nPreWriteLocation - m_nBeginIndex];
            node.m_pData = value;
            node.m_bWrite.store(true, std::memory_order_release);
        }
        inline bool IsLocationWriteSuccess(uint32_t nPreWriteLocation){
            return m_pPool[nPreWriteLocation - m_nBeginIndex].m_bWrite.load(std::memory_order_relaxed);
        }
        inline void PopLocation(T& value, uint32_t nPreWriteLocation){
            atomic_backoff bPause;
            StoreData& node = m_pPool[nPreWriteLocation - m_nBeginIndex];
            while(!node.m_bWrite.load(std::memory_order_acquire)){
                bPause.pause();
            }
            value = node.m_pData;
            node.m_bWrite.store(false, std::memory_order_relaxed);
        }
        inline bool IsLocationReadSuccess(uint32_t nPreWriteLocation){
            return m_pPool[nPreWriteLocation - m_nBeginIndex].m_bWrite.load(std::memory_order_relaxed) == false;
        }
    protected:
        std::atomic<Block*>     m_pNext = nullptr;
        uint32_t                m_nBeginIndex;
        StoreData               m_pPool[Traits::BlockSizeCount];
        friend class MicroQueue;
    };
    class MicroQueue{
    public:
        void Init(CCLockfreeQueue* pQueue){
            m_pQueue = pQueue;
            m_pWrite = pQueue->GetBlock(0);
            m_pRead = m_pWrite;
            m_nWriteLocation = 0;
            m_nReadLocation = 0;
        }
        void PushPosition(const T& value, uint32_t nPreWriteIndex){
            uint32_t nPreWriteLocation = nPreWriteIndex / Traits::ThreadWriteIndexModeIndex;
            atomic_backoff bPause;
            Block* pWriteBlock = nullptr;
            do{
                pWriteBlock = m_pWrite.load(std::memory_order_relaxed);
                uint32_t nDis = nPreWriteLocation - pWriteBlock->m_nBeginIndex;
                if(nDis == Traits::BlockSizeCount){
                    pWriteBlock = m_pQueue->GetBlock(nPreWriteIndex);
                    atomic_backoff bPauseWriteFinish;
                    //wait to preindex write finish
                    do{
                        if(m_nWriteLocation.load(std::memory_order_relaxed) == Traits::BlockSizeCount - 1){
                            //change write block
                            m_pWrite->m_pNext.store(pWriteBlock, std::memory_order_relaxed);
                            m_pWrite.store(pWriteBlock, std::memory_order_relaxed);
                            break;
                        }
                        bPauseWriteFinish.pause();
                    } while(true);
                    break;
                }
                else if(nDis < Traits::BlockSizeCount){
                    //inside
                    break;
                }
                bPause.pause();
            } while(true);
            pWriteBlock->PushLocation(value, nPreWriteLocation);
            MakePreWriteLocationSuccess(nPreWriteLocation);
        }
        bool PopPosition(T& value, uint32_t nReadIndex){
            uint32_t nReadLocation = nReadIndex / Traits::ThreadWriteIndexModeIndex;
            atomic_backoff bPause;
            Block* pReadBlock = nullptr;
            do{
                pReadBlock = m_pRead.load(std::memory_order_relaxed);
                uint32_t nDis = nReadLocation - pReadBlock->m_nBeginIndex;
                if(nDis == Traits::BlockSizeCount){
                    //wait
                    atomic_backoff bPauseWriteFinish;
                    do{
                        if(m_nReadLocation.load(std::memory_order_relaxed) == Traits::BlockSizeCount - 1){
                            //release block 
                            Block* pNextBlock = pReadBlock->m_pNext.load(std::memory_order_relaxed);
                            if(pNextBlock){
                                m_pRead.store(pNextBlock, std::memory_order_relaxed);
                                m_pQueue->ReleaseBlock(pReadBlock);
                                pReadBlock = pNextBlock;
                                break;
                            }
                        }
                        bPauseWriteFinish.pause();
                    } while(true);
                    break;
                }
                else if(nDis < Traits::BlockSizeCount){
                    //inside
                    break;
                }
                bPause.pause();
            } while(true);
            pReadBlock->PopLocation(value, nReadLocation);
            MakeReadLocationSuccess(nReadLocation);
        }
    protected:
        inline bool IsWriteSuccess(uint32_t nPreWriteLocation){
            Block* pWriteBlock = m_pWrite.load(std::memory_order_relaxed);
            uint32_t nDis = nPreWriteLocation - pWriteBlock->m_nBeginIndex;
            if(nDis < Traits::BlockSizeCount){
                return pWriteBlock->IsLocationWriteSuccess(nPreWriteLocation);
            }
            return false;
        }
        inline void MakePreWriteLocationSuccess(uint32_t nPreWriteLocation){
            if(m_nWriteLocation.compare_exchange_strong(nPreWriteLocation, nPreWriteLocation + 1, std::memory_order_acquire, std::memory_order_relaxed)){
                //write seq correct
                uint32_t nNowPreWriteLocation = m_pQueue->GetPreWriteIndex() / Traits::ThreadWriteIndexModeIndex;
                while(nNowPreWriteLocation != nPreWriteLocation){
                    nPreWriteLocation++;
                    //check nWritePosition + 1 is write finish
                    if(IsWriteSuccess(nPreWriteLocation)){
                        //write finish
                        if(!m_nWriteLocation.compare_exchange_strong(nPreWriteLocation, nPreWriteLocation + 1, std::memory_order_acquire, std::memory_order_relaxed)){
                            //add fail mean it add by self position, no need to continue
                            break;
                        }
                    }
                    else{
                        break;
                    }
                    nNowPreWriteLocation = m_pQueue->GetPreWriteIndex() / Traits::ThreadWriteIndexModeIndex;
                }
            }
        }
        inline bool IsReadSuccess(uint32_t nReadLocation){
            Block* pReadBlock = m_pRead.load(std::memory_order_relaxed);
            uint32_t nDis = nReadLocation - pReadBlock->m_nBeginIndex;
            if(nDis < Traits::BlockSizeCount){
                return pReadBlock->PopLocation(nReadLocation);
            }
            return false;
        }
        inline void MakeReadLocationSuccess(uint32_t nReadLocation){
            if(m_nReadLocation.compare_exchange_strong(nReadLocation, nReadLocation + 1, std::memory_order_acquire, std::memory_order_relaxed)){
                uint32_t nNowReadLocation = m_pQueue->GetReadIndex() / Traits::ThreadWriteIndexModeIndex;
                while(nNowReadLocation != nReadLocation){
                    nReadLocation++;
                    //check nWritePosition + 1 is write finish
                    if(IsReadSuccess(nReadLocation)){
                        //write finish
                        if(!m_nReadLocation.compare_exchange_strong(nReadLocation, nReadLocation + 1, std::memory_order_acquire, std::memory_order_relaxed)){
                            //add fail mean it add by self position, no need to continue
                            break;
                        }
                    }
                    else{
                        break;
                    }
                    nNowReadLocation = m_pQueue->GetReadIndex() / Traits::ThreadWriteIndexModeIndex;
                }
                
            }
        }
    protected:
        std::atomic<Block*>     m_pRead;
        std::atomic<Block*>     m_pWrite;
        std::atomic<uint32_t>   m_nWriteLocation;
        std::atomic<uint32_t>   m_nReadLocation;
        CCLockfreeQueue*        m_pQueue;
    };
    class BlockAlloc : public ObjectBaseClass{
    public:
        BlockAlloc(uint32_t nSize){
            m_nTotal = nSize;
            m_pCreate = new Block[m_nTotal];
            m_nGetSize = 0;
            m_pPre = nullptr;
        }
        virtual ~BlockAlloc(){
            delete[]m_pCreate;
        }
        inline Block* GetBlock(){
            if(m_nGetSize.load(std::memory_order_relaxed) >= m_nTotal){
                return nullptr;
            }
            auto index = m_nGetSize.fetch_add(1, std::memory_order_relaxed);
            return index < m_nTotal ? &m_pCreate[index] : nullptr;
        }
    protected:
        Block*                  m_pCreate;
        std::atomic<uint32_t>   m_nGetSize;
        uint32_t                m_nTotal;
        BlockAlloc*             m_pPre;
        friend class CCLockfreeQueue;
    };
public:
    CCLockfreeQueue(int nPreCreateBlockSize = Traits::ThreadWriteIndexModeIndex){
        for(int i = 0; i < Traits::ThreadWriteIndexModeIndex; i++){
            m_array[i].Init(this);
        }
        m_nNextQueueSize = nPreCreateBlockSize;
        m_pCurrentAllocate = new BlockAlloc(m_nNextQueueSize);
        m_nNextQueueSize *= Traits::BlockCountAddTimesNextTime;
        m_nReadIndex = 0;
        m_nPreWriteIndex = 0;
    }
    virtual ~CCLockfreeQueue(){
        BlockAlloc* pCheckAlloc = m_pCurrentAllocate.load(std::memory_order_relaxed);
        BlockAlloc* pFreeAlloc = nullptr;
        do{
            pFreeAlloc = pCheckAlloc;
            pCheckAlloc = pCheckAlloc->m_pPre;
            delete pFreeAlloc;
        } while(pCheckAlloc);
    }
    void Push(const T& value){
        uint32_t nPreWriteIndex = m_nPreWriteIndex.fetch_add(1, std::memory_order_relaxed);
        GetMicroQueueByIndex(nPreWriteIndex).PushPosition(value, nPreWriteIndex);
    }
    bool Pop(T& value){
        uint32_t nPreWriteIndex;
        uint32_t nRead;
        do{
            nRead = m_nReadIndex.load(std::memory_order_relaxed);
            nPreWriteIndex = m_nPreWriteIndex.load(std::memory_order_relaxed);
            if(nPreWriteIndex == nRead){
                // Queue is empty
                return false;
            }
            if(!m_nReadIndex.compare_exchange_strong(nRead, nRead + 1, std::memory_order_release, std::memory_order_relaxed)){
                continue;
            }
        } while(GetMicroQueueByIndex(nRead).PopPosition(value, nRead));
        return true;
    }
protected:
    inline MicroQueue& GetMicroQueueByIndex(uint32_t nIndex){
        return m_array[nIndex % Traits::ThreadWriteIndexModeIndex];
    }
    inline Block* GetBlock(uint32_t nWriteIndex){
        Block* pBlock = m_pStack.Pop();
        if(pBlock == nullptr){
            //get from create
            BlockAlloc* pAlloc = m_pCurrentAllocate.load(std::memory_order_relaxed);
            do{
                pBlock = pAlloc->GetBlock();
                if(pBlock == nullptr){
                    //need recreate
                    atomic_backoff bPause;
                    while(m_lock.exchange(1)){
                        bPause.pause();
                    }
                    BlockAlloc* pNowAlloc = m_pCurrentAllocate.load(std::memory_order_relaxed);
                    if(pNowAlloc == pAlloc){
                        BlockAlloc* pCreate = new BlockAlloc(m_nNextQueueSize);
                        m_nNextQueueSize *= Traits::BlockCountAddTimesNextTime;
                        pCreate->m_pPre = pAlloc;
                        m_pCurrentAllocate.store(pCreate, std::memory_order_relaxed);
                    }
                    m_lock.exchange(0);
                    pAlloc = pNowAlloc;
                    continue;
                }
            } while(false);
        }
        pBlock->Init(nWriteIndex);
        return pBlock;
    }
protected:
    MicroQueue                                                  m_array[Traits::ThreadWriteIndexModeIndex];
    basiclib::CCLockfreeStack                                   m_pStack;
    std::atomic<BlockAlloc*>                                    m_pCurrentAllocate;
    uint32_t                                                    m_nNextQueueSize;
    std::atomic<char>                                           m_lock;

    std::atomic<uint32_t>                                       m_nReadIndex;
    std::atomic<uint32_t>                                       m_nPreWriteIndex;
};
