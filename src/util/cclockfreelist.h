#pragma once

#define CCLockfreeListBaseObject basiclib::CBasicObject

class CCLockfreeStackNode{
public:
    CCLockfreeStackNode() : m_freeListNext(nullptr){

    }
private:
    std::atomic<CCLockfreeStackNode*> m_freeListNext;

    friend class CCLockfreeStack;
};


class CCLockfreeStack : public CCLockfreeListBaseObject{
public:
    CCLockfreeStack(){
        m_pHead = nullptr;
    }
    ~CCLockfreeStack(){

    }
    void Push(CCLockfreeStackNode* pNode){

    }
    CCLockfreeStackNode* Pop(){
        auto head = m_pHead.load(std::memory_order_acquire);
        while(head != nullptr){
            auto next = head->m_freeListNext.load(std::memory_order_relaxed);
            if(m_pHead.compare_exchange_strong(head, next, std::memory_order_release, std::memory_order_acquire)){
                return head;
            }
        }
        return nullptr;
    }
protected:
    std::atomic<CCLockfreeStackNode*> m_pHead;

};