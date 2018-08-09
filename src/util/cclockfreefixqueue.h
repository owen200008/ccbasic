/***********************************************************************************************
// 文件名：			cclockfreefixqueue.h
// 创建者：			Owen.Cai
// 创建时间:		2018/08/01 20:23:01
// 内容描述:		无锁stack
// 版本信息:		V1.0
 ************************************************************************************************/
#pragma once

__NS_BASIC_START

template<class T, uint32_t defaultfixsize = 32>
class CCLockfreeFixQueue : public basiclib::CBasicObject{
public:
    struct StoreLockfreeFixQueue{
        T                               m_data;
        std::atomic<uint8_t>            m_cWrite;
        StoreLockfreeFixQueue() : m_cWrite(0){
        }
    };
public:
    CCLockfreeFixQueue() : m_nWrite(0), m_nPreWrite(0), m_nPreRead(0), m_nRead(0){
        memset(m_pData, 0, sizeof(T) * defaultfixsize);
    }
    virtual ~CCLockfreeFixQueue(){

    }
    inline bool Push(const T& value){
        uint32_t nPreWriteCount = m_nPreWrite.fetch_add(1, std::memory_order_relaxed);
        StoreLockfreeFixQueue& node = m_pData[nPreWriteCount % defaultfixsize];
        uint8_t cWrite =  node.m_cWrite.fetch_or(0x10, std::memory_order_relaxed);
        if(cWrite & 0x10){
            //full
            return false;
        }
        node.m_data = value;
        node.m_cWrite.fetch_or(0x01, std::memory_order_release);
        addWritePositionSuccess(nPreWriteCount);
        return true;
    }
    inline bool Pop(T& value){
        /*uint32_t nWritePosition = m_nWrite.load(std::memory_order_relaxed);
        uint32_t nReadPosition = m_nRead.load(std::memory_order_relaxed);
        while(nReadPosition != nWritePosition){
            if(m_nRead.compare_exchange_strong(nReadPosition, nReadPosition + 1, std::memory_order_acquire, std::memory_order_relaxed)){
                StoreLockfreeFixQueue& node = m_pData[nReadPosition % defaultfixsize];
                value = node.m_data;
                node.m_cWrite.store(0, std::memory_order_relaxed);
                return true;
            }
            nWritePosition = m_nWrite.load(std::memory_order_relaxed);
        }
        return false;*/

        uint32_t nPreReadPosition = m_nPreRead.fetch_add(1, std::memory_order_relaxed);
        uint32_t nWritePosition = m_nWrite.load(std::memory_order_relaxed);
        if(nPreReadPosition >= nWritePosition){
            m_nPreRead.fetch_sub(1, std::memory_order_relaxed);
            //empty
            return false;
        }
        uint32_t nReadPosition = m_nRead.fetch_add(1, std::memory_order_relaxed);
        StoreLockfreeFixQueue& node = m_pData[nReadPosition % defaultfixsize];
        do{
            uint8_t cWrite = node.m_cWrite.load(std::memory_order_acquire);
            if(cWrite == 0x11){
                value = node.m_data;
                node.m_cWrite.store(0, std::memory_order_relaxed);
                return true;
            }
            else if(cWrite == 0x10){
                continue;
            }
        } while(false);
        printf("error pop");
        assert(0);
        return false;
    }
protected:
    inline bool isWritePosition(uint32_t nWritePosition){
        return (m_pData[nWritePosition % defaultfixsize].m_cWrite.load(std::memory_order_acquire) & 0x01) != 0;
    }
    void addWritePositionSuccess(uint32_t nWritePosition){
        //write success
        if(m_nWrite.compare_exchange_strong(nWritePosition, nWritePosition + 1, std::memory_order_acquire, std::memory_order_relaxed)){
            //write seq correct
            uint32_t nPreWritePosition = m_nPreWrite.load(std::memory_order_relaxed);
            while(nPreWritePosition != ++nWritePosition){
                //check nWritePosition + 1 is write finish
                if(isWritePosition(nWritePosition)){
                    //write finish
                    if(!m_nWrite.compare_exchange_strong(nWritePosition, nWritePosition + 1, std::memory_order_acquire, std::memory_order_relaxed)){
                        //add fail mean it add by self position, no need to continue
                        break;
                    }
                }
                else{
                    break;
                }
                nPreWritePosition = m_nPreWrite.load(std::memory_order_relaxed);
            }
        }
    }
protected:
    std::atomic<uint32_t>       m_nPreWrite;
    std::atomic<uint32_t>       m_nWrite;
    std::atomic<uint32_t>       m_nPreRead;
    std::atomic<uint32_t>       m_nRead;
    StoreLockfreeFixQueue       m_pData[defaultfixsize];
};

__NS_BASIC_END
