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

#pragma warning (push)
#pragma warning (disable: 4251)
#pragma warning (disable: 4275)
///////////////////////////////////////////////////////////////////////////////////////////////
// reserve high 8 bits for remote id
//
#define HANDLE_ID_BEGIN				0xff
#define HANDLE_ID_ALLOCATE_LOG		0xffffff
class _SCBASIC_DLL_API CCoroutineCtxHandle : public basiclib::CBasicObject
{
public:
	CCoroutineCtxHandle();
	virtual ~CCoroutineCtxHandle();

	//注册上下文
	uint32_t Register(CCoroutineCtx* pCtx);

	//注销上下文模块
	int UnRegister(CCoroutineCtx* pCtx);
	int UnRegister(uint32_t uHandleID);

	bool GrabContext(uint32_t handle, const std::function<void(CCoroutineCtx*)>& func);
	CRefCoroutineCtx GetContextByHandleID(uint32_t handle);
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
typedef basiclib::CBasicSingleton<CCoroutineCtxHandle> CSingletonCoroutineCtxHandle;

#pragma warning (pop)

#endif