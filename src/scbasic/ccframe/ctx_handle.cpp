#include "ctx_handle.h"
#include "log/ccframelog.h"

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
	basiclib::CBasicString& strCtxName = pCtx->GetCtxName();
	basiclib::CRWLockFunc lock(&m_lock, true, false);
	if (!strCtxName.IsEmpty()){
		if (m_mapNameToCtx.find(strCtxName) != m_mapNameToCtx.end()){
			return 0;
		}
		m_mapNameToCtx[strCtxName] = pCtx;
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
		basiclib::CBasicString& strName = ctx->GetCtxName();
		if (!strName.IsEmpty())
			m_mapNameToCtx.erase(strName);
		m_mapIDToCtx.erase(iter);
		
		m_releaseHandleID.push_back(uHandleID);
		lock.UnLockWrite();
		ret = 1;
	}
	return ret;
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
			CCFrameSCBasicLogEventErrorV("HandleID Allocate warning %d, need restart!!", uRetHandleID);
		}
	}

	return uRetHandleID;
}

