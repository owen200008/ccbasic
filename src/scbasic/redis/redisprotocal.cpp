#include "redisprotocal.h"
#include "read.h"

static redisReply *createReplyObject(int type){
    redisReply* r = (redisReply*)basiclib::BasicAllocate(sizeof(redisReply));
    if (r == NULL)
        return NULL;
    r->type = type;
	r->element = nullptr;
	r->str = nullptr;
    return r;
}
static void *createStringObject(const redisReadTask *task, char *str, size_t len){
    redisReply *r, *parent;
    char *buf;

    r = createReplyObject(task->type);
    if (r == NULL)
        return NULL;

    buf = (char*)basiclib::BasicAllocate(len + 1);
    if (buf == NULL) {
        freeReplyObject(r);
        return NULL;
    }

    assert(task->type == REDIS_REPLY_ERROR ||
        task->type == REDIS_REPLY_STATUS ||
        task->type == REDIS_REPLY_STRING);

    /* Copy string value */
    memcpy(buf, str, len);
    buf[len] = '\0';
    r->str = buf;
    r->len = len;

    if (task->parent) {
        parent = (redisReply*)task->parent->obj;
        assert(parent->type == REDIS_REPLY_ARRAY);
        parent->element[task->idx] = r;
    }
    return r;
}
static void *createArrayObject(const redisReadTask *task, int elements){
    redisReply *r, *parent;
    r = createReplyObject(REDIS_REPLY_ARRAY);
    if (r == NULL)
        return NULL;

    if (elements > 0) {
        r->element = (redisReply**)basiclib::BasicAllocate(elements * sizeof(redisReply*));
        if (r->element == NULL) {
            freeReplyObject(r);
            return NULL;
        }
    }

    r->elements = elements;

    if (task->parent) {
        parent = (redisReply*)task->parent->obj;
        assert(parent->type == REDIS_REPLY_ARRAY);
        parent->element[task->idx] = r;
    }
    return r;
}
static void *createIntegerObject(const redisReadTask *task, long long value){
    redisReply *r, *parent;

    r = createReplyObject(REDIS_REPLY_INTEGER);
    if (r == NULL)
        return NULL;

    r->integer = value;

    if (task->parent) {
        parent = (redisReply*)task->parent->obj;
        assert(parent->type == REDIS_REPLY_ARRAY);
        parent->element[task->idx] = r;
    }
    return r;
}
static void *createNilObject(const redisReadTask *task){
    redisReply *r, *parent;
    r = createReplyObject(REDIS_REPLY_NIL);
    if (r == NULL)
        return NULL;

    if (task->parent) {
        parent = (redisReply*)task->parent->obj;
        assert(parent->type == REDIS_REPLY_ARRAY);
        parent->element[task->idx] = r;
    }
    return r;
}

/* Free a reply object */
void freeReplyObject(void *reply) {
    redisReply* r = (redisReply*)reply;
    size_t j;

    if (r == NULL)
        return;

    switch (r->type) {
    case REDIS_REPLY_INTEGER:
        break; /* Nothing to free */
    case REDIS_REPLY_ARRAY:
        if (r->element != NULL) {
            for (j = 0; j < r->elements; j++)
                if (r->element[j] != NULL)
                    freeReplyObject(r->element[j]);
            basiclib::BasicDeallocate(r->element);
        }
        break;
    case REDIS_REPLY_ERROR:
    case REDIS_REPLY_STATUS:
    case REDIS_REPLY_STRING:
        if (r->str != NULL)
            basiclib::BasicDeallocate(r->str);
        break;
    }
    basiclib::BasicDeallocate(r);
}
redisReplyObjectFunctions g_defaultResidFunc = {
    createStringObject,
    createArrayObject,
    createIntegerObject,
    createNilObject,
    freeReplyObject
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
CRedisSendPacket::CRedisSendPacket(int nReVert)
{
    m_data.reserve(nReVert);
    m_bSelf = false;
    m_pNext = nullptr;
}

CRedisSendPacket::~CRedisSendPacket()
{
    if (m_bSelf){
        for (auto& data : m_data){
            delete data;
        }
        if (m_pNext){
            delete m_pNext;
        }
    }
}

void CRedisSendPacket::PushBasicValue(basiclib::CNetBasicValue& value)
{
    basiclib::CNetBasicValue* pPush = nullptr;
    if (m_bSelf){
        pPush = new basiclib::CNetBasicValue();
        *pPush = value;
    }
    else{
        pPush = &value;
    }
    m_data.push_back(pPush);
}
void CRedisSendPacket::PushNextSendPacket(CRedisSendPacket& packet){
    if (m_bSelf){
        m_pNext = new CRedisSendPacket();
        *m_pNext = packet;
        m_pNext->ChangeToDataSafe();
    }
    else{
        m_pNext = &packet;
    }
}

void CRedisSendPacket::ChangeToDataSafe()
{
    if (!m_bSelf){
        int nSize = m_data.size();
        for (int i = 0; i < nSize; i++){
            basiclib::CNetBasicValue* pPush = new basiclib::CNetBasicValue();
            *pPush = *(m_data[i]);
            std::swap(pPush, m_data[i]);
        }
        m_bSelf = true;
        if (m_pNext){
            m_pNext->ChangeToDataSafe();
        }
    }
}

void CRedisSendPacket::DataToSmartBuffer(basiclib::CBasicSmartBuffer& smBuf)
{
    smBuf.SetDataLength(0);
    //size
#define MAX_SPRINT_SIZE 64
    char szBuf[MAX_SPRINT_SIZE] = { 0 };
    int nFormatLength = sprintf(szBuf, "*%d\r\n", m_data.size());
    smBuf.AppendData(szBuf, nFormatLength);
    for (auto& data : m_data){
        basiclib::CNetBasicValue* pPush = data;
        const char* pCopyString = nullptr;
        int nCopyLength = 0;

        int nBasicValueLength = pPush->GetDataLength();
        switch (pPush->GetDataType()){
        case basiclib::NETVALUE_LONG:
        {
            nFormatLength = sprintf(szBuf, "%d", pPush->GetLong());
            pCopyString = szBuf;
        }
        break;
        case basiclib::NETVALUE_DOUBLE:
        {
            nFormatLength = sprintf(szBuf, "%f", pPush->GetDouble());
            pCopyString = szBuf;
        }
        break;
        case basiclib::NETVALUE_LONGLONG:
        {
#ifdef __BASICWINDOWS
            nFormatLength = sprintf(szBuf, "%I64d", pPush->GetLongLong());
#else
            nFormatLength = sprintf(szBuf, "%lld", pPush->GetLongLong());
#endif
            pCopyString = szBuf;
        }
        break;
        case basiclib::NETVALUE_CHAR:
        {
            pCopyString = pPush->GetStringRef();
            nFormatLength = strlen(pCopyString);
        }
        break;
        case basiclib::NETVALUE_STRING:
        {
            pCopyString = pPush->GetStringRef();
            nFormatLength = pPush->GetDataLength();
        }
        break;
        }
        if (pCopyString && nFormatLength > 0){
            int nSmBufLength = smBuf.GetDataLength();
            //保证长度足够
            smBuf.SetDataLength(nSmBufLength + nFormatLength + 64);
            char* pBegin = smBuf.GetDataBuffer() + nSmBufLength;
            //size
            int nSize = sprintf(pBegin, "$%d\r\n%s\r\n", nFormatLength, pCopyString);
            //设置最终的长度
            smBuf.SetDataLength(nSmBufLength + nSize);
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////////////
CRedisReplyPacket::CRedisReplyPacket()
{
    m_pReply = nullptr;
    m_pNext = nullptr;
}

CRedisReplyPacket::~CRedisReplyPacket()
{
    if (m_pReply){
        freeReplyObject(m_pReply);
    }
}
void CRedisReplyPacket::BindReply(redisReply* pReply)
{
    if (m_pReply){
        freeReplyObject(m_pReply);
    }
    m_pReply = pReply;
}

