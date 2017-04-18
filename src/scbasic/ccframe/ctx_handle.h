/***********************************************************************************************
// 文件名:     ctx_handle.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 内容描述:   协程资源基类
// 版本信息:   1.0V
************************************************************************************************/
#ifndef SCBASIC_CTX_HANDLE_H
#define SCBASIC_CTX_HANDLE_H

#include "coroutine_ctx.h"

///////////////////////////////////////////////////////////////////////////////////////////////
// reserve high 8 bits for remote id
//
#define HANDLE_ID_BEGIN				0xff
#define HANDLE_ID_ALLOCATE_LOG		0xffffff
class CCoroutineCtxHandle : public basiclib::CBasicObject
{
public:
    static CCoroutineCtxHandle* GetInstance();

	CCoroutineCtxHandle();
	virtual ~CCoroutineCtxHandle();

	//注册上下文
	uint32_t Register(CCoroutineCtx* pCtx);

	//注销上下文模块
	int UnRegister(CCoroutineCtx* pCtx);
	int UnRegister(uint32_t uHandleID);

	bool GrabContext(uint32_t handle, const std::function<void(CCoroutineCtx*)>& func);
	CRefCoroutineCtx GetContextByHandleID(uint32_t handle);

    //! 根据名字查找ctxid
    uint32_t GetCtxIDByName(const char* pName);
    //! 遍历
    void Foreach(const std::function<bool(CRefCoroutineCtx)>& func){
        basiclib::CRWLockFunc lock(&m_lock, false, true);
        for (auto& iter : m_mapIDToCtx){
            if (!func(iter.second))
                return;
        }
    }
protected:
	uint32_t GetNewHandleID();
protected:
	basiclib::RWLock		m_lock;
	uint32_t				m_handle_index;

	typedef basiclib::basic_vector<uint32_t>	ReleaseNodeHandleID;
	ReleaseNodeHandleID		m_releaseHandleID;

	typedef basiclib::basic_map<basiclib::CBasicString, CRefCoroutineCtx>	MapNameToCtx;
	typedef basiclib::basic_map<uint32_t, CRefCoroutineCtx>					MapHandleIDToCtx;
	MapNameToCtx		m_mapNameToCtx;
	MapHandleIDToCtx	m_mapIDToCtx;
};


#endif