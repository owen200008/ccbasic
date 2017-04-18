#include "ctx_handle.h"
#include "log/ctx_log.h"

static CCoroutineCtxHandle m_gHandle;
CCoroutineCtxHandle* CCoroutineCtxHandle::GetInstance()
{
    return &m_gHandle;
}

CCoroutineCtxHandle::CCoroutineCtxHandle()
{
	m_handle_index = HANDLE_ID_BEGIN;
	m_releaseHandleID.reserve(0xffff);
}

CCoroutineCtxHandle::~CCoroutineCtxHandle()
{

}

uint32_t CCoroutineCtxHandle::Register(CCoroutineCtx* pCtx)
{
	const char* pCtxName = pCtx->GetCtxName();
	basiclib::CRWLockFunc lock(&m_lock, true, false);
    if (pCtxName){
        if (m_mapNameToCtx.find(pCtxName) != m_mapNameToCtx.end()){
			return 0;
		}
        m_mapNameToCtx[pCtxName] = pCtx;
	}
	uint32_t uHandleID = GetNewHandleID();
	m_mapIDToCtx[uHandleID] = pCtx;
	pCtx->m_ctxID = uHandleID;
	return uHandleID;
}

int CCoroutineCtxHandle::UnRegister(CCoroutineCtx* pCtx)
{
	uint32_t uHandleID = pCtx->GetCtxID();
	return UnRegister(uHandleID);
}

int CCoroutineCtxHandle::UnRegister(uint32_t uHandleID)
{
	int ret = 0;
	basiclib::CRWLockFunc lock(&m_lock, true, false);
	MapHandleIDToCtx::iterator iter = m_mapIDToCtx.find(uHandleID);
	if (iter != m_mapIDToCtx.end())
	{
		CRefCoroutineCtx ctx = iter->second;
		const char* pCtxName = ctx->GetCtxName();
        if (pCtxName)
            m_mapNameToCtx.erase(pCtxName);
		m_mapIDToCtx.erase(iter);
		
		m_releaseHandleID.push_back(uHandleID);
		lock.UnLockWrite();
		ret = 1;
	}
	return ret;
}

//! 根据名字查找ctxid
uint32_t CCoroutineCtxHandle::GetCtxIDByName(const char* pName)
{
    basiclib::CRWLockFunc lock(&m_lock, false, true);
    MapNameToCtx::iterator iter = m_mapNameToCtx.find(pName);
    if (iter != m_mapNameToCtx.end())
        return iter->second->GetCtxID();
    return 0;
}

bool CCoroutineCtxHandle::GrabContext(uint32_t handle, const std::function<void(CCoroutineCtx*)>& func)
{
	CRefCoroutineCtx pRet = GetContextByHandleID(handle);
	if (pRet != nullptr)
	{
		func(pRet.GetResFunc());
	}
	return pRet != nullptr;
}
CRefCoroutineCtx CCoroutineCtxHandle::GetContextByHandleID(uint32_t handle)
{
	basiclib::CRWLockFunc lock(&m_lock, false, true);
	MapHandleIDToCtx::iterator iter = m_mapIDToCtx.find(handle);
	if (iter != m_mapIDToCtx.end())
	{
		return iter->second;
	}
	return nullptr;
}

uint32_t CCoroutineCtxHandle::GetNewHandleID()
{
	int nSize = m_releaseHandleID.size();
	if (nSize > 0)
	{
		uint32_t uRetHandleID = m_releaseHandleID.back();
		m_releaseHandleID.pop_back();
		return uRetHandleID;
	}
	uint32_t uRetHandleID = ++m_handle_index;
	if (uRetHandleID > HANDLE_ID_ALLOCATE_LOG)
	{
		if (uRetHandleID % 10000 == 0)
		{
			//100 log
            CCFrameSCBasicLogEventErrorV(nullptr, "HandleID Allocate warning %d, need restart!!", uRetHandleID);
		}
	}

	return uRetHandleID;
}

