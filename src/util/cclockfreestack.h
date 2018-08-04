/***********************************************************************************************
 // 文件名:     cclockfreestack.h
 // 创建者:     蔡振球
 // Email:     zqcai@w.cn
 // 创建时间:   2018/8/1 20:23:31
 // 内容描述:   无锁stack
 // 版本信息:   1.0V
 ************************************************************************************************/
#pragma once

__NS_BASIC_START

class CCLockfreeStackNode{
public:
    CCLockfreeStackNode(){
        m_freeListNext = nullptr;
        m_freeListRefs = 0;
    }
private:
    std::atomic<std::uint32_t>          m_freeListRefs;
    std::atomic<CCLockfreeStackNode*>   m_freeListNext;

    friend class CCLockfreeStack;
};

class CCLockfreeStack : public basiclib::CBasicObject{
    static const std::uint32_t REFS_MASK = 0x7FFFFFFF;
    static const std::uint32_t SHOULD_BE_ON_FREE = 0x80000000;
public:
    CCLockfreeStack(){
        m_pHead = nullptr;
    }
    virtual ~CCLockfreeStack(){

    }
    inline void Push(CCLockfreeStackNode* pNode){
        if(pNode->m_freeListRefs.fetch_add(SHOULD_BE_ON_FREE, std::memory_order_acq_rel) == 0){
            PushByRefCountZero(pNode);
        }
    }
    inline CCLockfreeStackNode* Pop(){
        auto head = m_pHead.load(std::memory_order_acquire);
        while(head != nullptr){
            auto prevHead = head;
            auto refs = head->m_freeListRefs.load(std::memory_order_relaxed);
            if((refs & REFS_MASK) == 0 || !head->m_freeListRefs.compare_exchange_strong(refs, refs + 1, std::memory_order_acquire, std::memory_order_relaxed)){
                head = m_pHead.load(std::memory_order_acquire);
                continue;
            }
            auto next = head->m_freeListNext.load(std::memory_order_relaxed);
            if(m_pHead.compare_exchange_strong(head, next, std::memory_order_acquire, std::memory_order_relaxed)){
                // Decrease refcount twice, once for our ref, and once for the list's ref
                head->m_freeListRefs.fetch_sub(2, std::memory_order_release);
                return head;
            }
            refs = prevHead->m_freeListRefs.fetch_sub(1, std::memory_order_acq_rel);
            if(refs == SHOULD_BE_ON_FREE + 1){
                PushByRefCountZero(prevHead);
            }
        }
        return nullptr;
    }
protected:
    inline void PushByRefCountZero(CCLockfreeStackNode* pNode){
        auto head = m_pHead.load(std::memory_order_relaxed);
        while(true){
            pNode->m_freeListNext.store(head, std::memory_order_relaxed);
            pNode->m_freeListRefs.store(1, std::memory_order_release);
            if(!m_pHead.compare_exchange_strong(head, pNode, std::memory_order_release, std::memory_order_relaxed)){
                // Hmm, the add failed, but we can only try again when the refcount goes back to zero
                if(pNode->m_freeListRefs.fetch_add(SHOULD_BE_ON_FREE - 1, std::memory_order_release) == 1){
                    continue;
                }
            }
            return;
        }
    }
protected:
    std::atomic<CCLockfreeStackNode*> m_pHead;
};

__NS_BASIC_END
