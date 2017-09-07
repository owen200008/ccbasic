#ifndef BASICLIB_REDISPROTOCAL_H
#define BASICLIB_REDISPROTOCAL_H

#include <basic.h>
#include "../scbasic_head.h"
#include "../scbasic/redis/read.h"

#pragma warning (push)
#pragma warning (disable: 4251)
#pragma warning (disable: 4275)
class _SCBASIC_DLL_API CRedisSendPacket : basiclib::CBasicObject
{
public:
    CRedisSendPacket(int nReVert = 4);
    virtual ~CRedisSendPacket();

    void PushBasicValue(basiclib::CNetBasicValue& value);
    void ChangeToDataSafe();

    void DataToSmartBuffer(basiclib::CBasicSmartBuffer& smBuf);

    CRedisSendPacket* GetNextSendPacket(){ return m_pNext; }
    void PushNextSendPacket(CRedisSendPacket& packet);
protected:
    typedef basiclib::basic_vector<basiclib::CNetBasicValue*> VTRedisData;
    VTRedisData                     m_data;
    bool                            m_bSelf;
    CRedisSendPacket*               m_pNext;
};

typedef struct redisReply {
    int type; /* REDIS_REPLY_* */
    long long integer; /* The integer when type is REDIS_REPLY_INTEGER */
    size_t len; /* Length of string */
    char *str; /* Used for both REDIS_REPLY_ERROR and REDIS_REPLY_STRING */
    size_t elements; /* number of elements, for REDIS_REPLY_ARRAY */
    struct redisReply **element; /* elements vector for REDIS_REPLY_ARRAY */
} redisReply;

class _SCBASIC_DLL_API CRedisReplyPacket : basiclib::CBasicObject
{
public:
    CRedisReplyPacket();
    virtual ~CRedisReplyPacket();

    void BindReply(redisReply* pReply);
    redisReply* GetReply(){ return m_pReply; }

    CRedisReplyPacket* GetNextReplyPacket(){ return m_pNext; }
    void PushNextReplyPacket(CRedisReplyPacket& packet){
        m_pNext = &packet;
    }
protected:
    redisReply*         m_pReply;
    CRedisReplyPacket*  m_pNext;
};
/* Free a reply object */
_SCBASIC_DLL_API void freeReplyObject(void *reply);

#pragma warning (pop)

#endif
