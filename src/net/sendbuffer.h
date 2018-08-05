/***********************************************************************************************
// 文件名:     sendbuffer.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012-2-22 22:55:08
// 内容描述:   定义发送数据缓冲和发送队列
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_SENDBUFFER_H
#define BASIC_SENDBUFFER_H

#include <time.h>
#include <deque>
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

/////////////////////////////////////////////////////////////////////////////////////////////
//声明
////////////////////////////////////////////////////////////////////////////////////////////
__NS_BASIC_START
///////////////////////////////////////////////////////////////////////////////
//发送的buffer
#pragma warning (push)
#pragma warning (disable: 4200)
struct SendBufferCache{
    int32_t		m_cbData;
    char		m_cRevertData[0];
    static SendBufferCache* CreateCache(int32_t nLength){
        SendBufferCache* pRet = (SendBufferCache*)basiclib::BasicAllocate(sizeof(SendBufferCache) + nLength);
        pRet->m_cbData = nLength;
        return pRet;
    }
    static SendBufferCache* CreateCache(void* pBuffer, int32_t nLength){
        SendBufferCache* pRet = (SendBufferCache*)basiclib::BasicAllocate(sizeof(SendBufferCache) + nLength);
        pRet->m_cbData = nLength;
        memcpy(pRet->m_cRevertData, pBuffer, nLength);
        return pRet;
    }
    static void ReleaseCache(SendBufferCache* pCache){
        basiclib::BasicDeallocate(pCache);
    }
};
#pragma warning (pop)

struct SendBufferCacheMgr{
    SendBufferCache* m_pCache;
    SendBufferCacheMgr(){
        m_pCache = nullptr;
    }
    ~SendBufferCacheMgr(){
        if(m_pCache)
            SendBufferCache::ReleaseCache(m_pCache);
    }
    SendBufferCacheMgr(void* pBuffer, int32_t cbData){
        m_pCache = SendBufferCache::CreateCache(pBuffer, cbData);
    }
    void Reset(const char* pBuffer, int32_t cbData){
        char* pData = ResetDataLength(cbData);
        memcpy(pData, pBuffer, cbData);
    }
    char* ResetDataLength(int32_t cbData){
        if(m_pCache){
            SendBufferCache::ReleaseCache(m_pCache);
        }
        m_pCache = SendBufferCache::CreateCache(cbData);
        return m_pCache->m_cRevertData;
    }
    //转移出去,外部释放
    SendBufferCache* SwapCache(){
        SendBufferCache* pCache = m_pCache;
        m_pCache = nullptr;
        return pCache;
    }
};

class CMsgSendBuffer : public basiclib::CBasicSmartBuffer{
public:
    CMsgSendBuffer();
    virtual ~CMsgSendBuffer();

    //返回剩余的长度
    int32_t SendBuffer(int32_t lSend);
};

__NS_BASIC_END
//////////////////////////////////////////////////////////////////////////////////////////////////

#endif 
