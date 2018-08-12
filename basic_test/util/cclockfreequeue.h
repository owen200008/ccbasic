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
    //! block size, 2的指数
    static const uint16_t BlockDefaultPerSize = 16;
    //! 下一次分配的增大的倍数
    static const uint32_t BlockCountAddTimesNextTime = 2;
    //! 分配开始时候的index
    static const uint32_t CCLockfreeQueueStartIndex = 0;

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
    basiclib::CBasicThreadTLS& GetTLS() {
        return m_tls;
    }
protected:
    basiclib::CBasicThreadTLS    m_tls;
};


template<class T, class Traits = CCLockfreeQueueFunc, class ObjectBaseClass = CCLockfreeObject<Traits>>
class CCLockfreeQueue : public ObjectBaseClass{
public:
    struct Block{
    public:
        struct StoreData{
            T                           m_pData;
            std::atomic<uint8_t>        m_bWrite;
        };
        std::atomic<Block*>     m_pNext;
        uint32_t                m_nBeginIndex;
        uint32_t                m_nSize;
        StoreData               m_pPool[];
    public:
        static Block* CreateBlock(uint32_t nSize) {
            Block* pBlock = (Block*)Traits::malloc(sizeof(Block) + nSize * sizeof(StoreData));
            pBlock->m_nSize = nSize;
            return pBlock;
        }
        static void ReleaseBlock(Block* p) {
            Traits::free(p);
        }
        void Init(uint32_t nBeginIndex = 0){
            //初始化 必然会有store writeindex 那边做release保证其他地方拿到的beginidnex正确
            m_nBeginIndex = nBeginIndex;
            m_pNext.store(nullptr, std::memory_order_relaxed);
            memset(m_pPool, 0, sizeof(StoreData) * m_nSize);
        }
        //must can full call
        inline void IsWriteFull(){
            atomic_backoff bPauseWriteFinish;
            uint32_t i = 0;
            while(i < m_nSize){
                if(m_pPool[i].m_bWrite.load(std::memory_order_relaxed) & 0x10){
                    i++;
                }
                else{
                    bPauseWriteFinish.pause();
                }
            }
        }
        inline void PushLocation(const T& value, uint32_t nPreWriteLocation){
            StoreData& node = m_pPool[nPreWriteLocation - m_nBeginIndex];
            node.m_pData = value;
            node.m_bWrite.store(0x11, std::memory_order_release);
        }
        inline void IsReadEmpty(){
            atomic_backoff bPauseWriteFinish;
            uint32_t i = 0;
            while (i < m_nSize) {
                if (m_pPool[i].m_bWrite.load(std::memory_order_relaxed) == 0x10) {
                    i++;
                }
                else {
                    bPauseWriteFinish.pause();
                }
            }
        }
        inline void PopLocation(T& value, uint32_t nPreWriteLocation){
            atomic_backoff bPause;
            StoreData& node = m_pPool[nPreWriteLocation - m_nBeginIndex];
            while(!(node.m_bWrite.load(std::memory_order_acquire) & 0x01)){
                bPause.pause();
            }
            value = node.m_pData;
            node.m_bWrite.store(0x10, std::memory_order_relaxed);
        }
        inline bool IsLocationReadSuccess(uint32_t nPreWriteLocation){
            return m_pPool[nPreWriteLocation - m_nBeginIndex].m_bWrite.load(std::memory_order_relaxed) == false;
        }
    };

    class MicroQueue{
    public:
        MicroQueue() {
            
        }
        virtual ~MicroQueue() {
            Block* p = m_pRevertBlock.load();
            if (p != nullptr) {
                Block::ReleaseBlock(p);
            }
        }
        void Init(CCLockfreeQueue* pQueue){
            m_pQueue = pQueue;
            uint32_t nSetBeginIndex = (Traits::CCLockfreeQueueStartIndex / Traits::BlockDefaultPerSize / Traits::ThreadWriteIndexModeIndex) * Traits::BlockDefaultPerSize;
            Block* pBlock = Block::CreateBlock(Traits::BlockDefaultPerSize);
            pBlock->Init(nSetBeginIndex);
            m_pWrite = pBlock;
            m_pRead = pBlock;
            m_pRevertBlock = nullptr;
        }
        inline void PushPosition(const T& value, uint32_t nPreWriteIndex){
            uint32_t nPreWriteLocation = nPreWriteIndex / Traits::ThreadWriteIndexModeIndex;
            atomic_backoff bPause;
            Block* pWriteBlock = m_pWrite.load(std::memory_order_acquire);
            do {
                uint32_t nDis = nPreWriteLocation - pWriteBlock->m_nBeginIndex;
                if (nDis == pWriteBlock->m_nSize || (nPreWriteLocation == 0 && pWriteBlock->m_nBeginIndex != 0)) {
                    Block* pGetBlock = m_pRevertBlock.exchange(nullptr, std::memory_order_relaxed);
                    if (pGetBlock == nullptr) {
                        pGetBlock = Block::CreateBlock(pWriteBlock->m_nSize * 2);
                    }
                    pGetBlock->Init(nPreWriteLocation);
                    //change write block, 这边需要release，因为读的时候读取到next需要同步next的信息
                    pWriteBlock->m_pNext.store(pGetBlock, std::memory_order_release);

                    pGetBlock->PushLocation(value, nPreWriteLocation);
                    //wait to preindex write finish
                    pWriteBlock->IsWriteFull();
                    atomic_backoff bPauseWriteFinish;
                    Block* pReadyWrite = nullptr;
                    for (;;) {
                        pReadyWrite = pWriteBlock;
                        //if know how to 
                        if (m_pWrite.compare_exchange_strong(pReadyWrite, pGetBlock, std::memory_order_acquire, std::memory_order_relaxed)) {
                            break;
                        }
                        bPauseWriteFinish.pause();
                    }
                    return;
                }
                else if (nDis < pWriteBlock->m_nSize) {
                    break;
                }
                else {
                    Block* pNextBlock = pWriteBlock->m_pNext.load(std::memory_order_acquire);
                    while (pNextBlock == nullptr) {
                        bPause.pause();
                        pNextBlock = pWriteBlock->m_pNext.load(std::memory_order_acquire);
                    }
                    pWriteBlock = pNextBlock;
                }
            } while (true);
            pWriteBlock->PushLocation(value, nPreWriteLocation);
        }
        inline void PopPosition(T& value, uint32_t nReadIndex){
            uint32_t nReadLocation = nReadIndex / Traits::ThreadWriteIndexModeIndex;
            Block* pReadBlock = m_pRead.load(std::memory_order_relaxed);
            atomic_backoff bPauseGetNextFinish;
            do{
                uint32_t nDis = nReadLocation - pReadBlock->m_nBeginIndex;
                if (nDis == pReadBlock->m_nSize || (nReadLocation == 0 && pReadBlock->m_nBeginIndex != 0)) {
                    //read first use cpu time
                    Block* pNextBlock = pReadBlock->m_pNext.load(std::memory_order_acquire);
                    while (pNextBlock == nullptr) {
                        bPauseGetNextFinish.pause();
                        pNextBlock = pReadBlock->m_pNext.load(std::memory_order_acquire);
                    }
                    pNextBlock->PopLocation(value, nReadLocation);

                    //wait to read finish
                    pReadBlock->IsReadEmpty();
                    atomic_backoff bPauseReadFinish;
                    Block* pReadyRead = nullptr;
                    for (;;) {
                        pReadyRead = pReadBlock;
                        //if know how to 
                        if (m_pRead.compare_exchange_strong(pReadyRead, pNextBlock, std::memory_order_acquire, std::memory_order_relaxed)) {
                            break;
                        }
                        bPauseReadFinish.pause();
                    }
                    Block* pBlock = m_pRevertBlock.exchange(pReadBlock, std::memory_order_relaxed);
                    if (pBlock != nullptr) {
                        Block::ReleaseBlock(pBlock);
                    }
                    return;
                }
                else if (nDis < pReadBlock->m_nSize) {
                    //inside
                    break;
                }
                else {
                    Block* pNextBlock = pReadBlock->m_pNext.load(std::memory_order_acquire);
                    while (pNextBlock == nullptr) {
                        bPauseGetNextFinish.pause();
                        pNextBlock = pReadBlock->m_pNext.load(std::memory_order_acquire);
                    }
                    pReadBlock = pNextBlock;
                }
            } while(true);
            pReadBlock->PopLocation(value, nReadLocation);
        }
    protected:
        std::atomic<Block*>     m_pRevertBlock;
        std::atomic<Block*>     m_pRead;
        std::atomic<Block*>     m_pWrite;
        CCLockfreeQueue*        m_pQueue;
    };
public:
    CCLockfreeQueue(){
        m_tls.CreateTLS();
        uint32_t nSetBeginIndex = (Traits::CCLockfreeQueueStartIndex / Traits::BlockDefaultPerSize / Traits::ThreadWriteIndexModeIndex) * Traits::BlockDefaultPerSize * Traits::ThreadWriteIndexModeIndex;
        m_nReadIndex = nSetBeginIndex;
        m_nPreWriteIndex = nSetBeginIndex;
        for(int i = 0; i < Traits::ThreadWriteIndexModeIndex; i++){
            m_array[i].Init(this);
        }
    }
    virtual ~CCLockfreeQueue(){
    }
    void Push(const T& value){
        uint32_t nPreWriteIndex = m_nPreWriteIndex.fetch_add(1, std::memory_order_relaxed);
        m_array[nPreWriteIndex%Traits::ThreadWriteIndexModeIndex].PushPosition(value, nPreWriteIndex);
    }
    bool Pop(T& value){
        uint32_t nPreWriteIndex;
        uint32_t nRead = m_nReadIndex.load(std::memory_order_relaxed);
        do{
            nPreWriteIndex = m_nPreWriteIndex.load(std::memory_order_relaxed);
            if(nPreWriteIndex == nRead){
                // Queue is empty
                return false;
            }
            if(!m_nReadIndex.compare_exchange_strong(nRead, nRead + 1, std::memory_order_acquire, std::memory_order_relaxed)){
                continue;
            }
            m_array[nRead%Traits::ThreadWriteIndexModeIndex].PopPosition(value, nRead);
            return true;
        } while(true);
        return false;
    }
    inline uint32_t GetReadIndex(){
        return m_nReadIndex.load(std::memory_order_relaxed);
    }
    inline uint32_t GetPreWriteIndex(){
        return m_nPreWriteIndex.load(std::memory_order_relaxed);
    }
protected:
    MicroQueue                                                  m_array[Traits::ThreadWriteIndexModeIndex];
    std::atomic<uint32_t>                                       m_nReadIndex;
    std::atomic<uint32_t>                                       m_nPreWriteIndex;
};
