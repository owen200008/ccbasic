#ifndef INC_COROUTINEPLUS_H
#define INC_COROUTINEPLUS_H

#include "libco_coroutine.h"

enum CoroutineState
{
	CoroutineState_Dead 	= 0,
	CoroutineState_Ready 	= 1,
	CoroutineState_Running	= 2,
	CoroutineState_Suspend	= 3,
};

#define STACK_SIZE 			(1024*1024)
#define DEFAULT_COROUTINE 	16
#define RESUME_MAXPARAM		4
class CCorutinePlus;
class CCorutinePlusPool;
typedef void (*coroutine_func)(CCorutinePlus* pCorutinePlus);
class CCorutinePlus : public basiclib::CBasicObject
{
public:
	CCorutinePlus();
	virtual ~CCorutinePlus();
	
	void CheckStackSize(int nDefaultStackSize);
	void ReInit(coroutine_func func);
	void YieldCorutine();
	template<class T>
	T GetResumeParam(int nParam)
	{
		if(nParam < 0 || nParam >= RESUME_MAXPARAM)
			nParam = 0;
		void* pPTR = m_pResumeParam[nParam];
		return *((T*)pPTR);
	}
	template<class T>
	T* GetResumeParamPoint(int nParam)
	{
		if(nParam < 0 || nParam >= RESUME_MAXPARAM)
			nParam = 0;
		void* pPTR = m_pResumeParam[nParam];
		return (T*)pPTR;
	}

	void Resume(CCorutinePlusPool* pPool);
	template<class T1>
	void Resume(CCorutinePlusPool* pPool, T1& param1)
	{
		m_pResumeParam[0] = &param1;
		Resume(pPool);
	}
	template<class T1, class T2>
	void Resume(CCorutinePlusPool* pPool, T1& param1, T2& param2)
	{
		m_pResumeParam[0] = &param1;
		m_pResumeParam[1] = &param2;
		Resume(pPool);
	}
	template<class T1, class T2, class T3>
	void Resume(CCorutinePlusPool* pPool, T1& param1, T2& param2, T3& param3)
	{
		m_pResumeParam[0] = &param1;
		m_pResumeParam[1] = &param2;
		m_pResumeParam[2] = &param3;
		Resume(pPool);
	}
	template<class T1, class T2, class T3, class T4>
	void Resume(CCorutinePlusPool* pPool, T1& param1, T2& param2, T3& param3, T4& param4)
	{
		m_pResumeParam[0] = &param1;
		m_pResumeParam[1] = &param2;
		m_pResumeParam[2] = &param3;
		m_pResumeParam[3] = &param4;
		Resume(pPool);
	}
	CCorutinePlusPool* GetRunPool(){return m_pRunPool;}

	//no call self
	void StartFunc();
	void StartFuncLibco();
protected:
	CoroutineState							m_state;
	coroutine_func 							m_func;
#ifdef USE_UCONTEXT
	ucontext_t 								m_ctx;
#else
	coctx_t									m_ctx;
#endif
	char*									m_stack;
	int										m_nCap;
	int										m_nSize;

	//resume param
	CCorutinePlusPool*						m_pRunPool;
	void*									m_pResumeParam[RESUME_MAXPARAM];
};

//thread not safe
class CCorutinePlusPool : public basiclib::CBasicObject
{
public:
	CCorutinePlusPool();
	virtual ~CCorutinePlusPool();

	bool InitCorutine(int nDefaultSize = DEFAULT_COROUTINE, int nDefaultStackSize = 1024 * 16);
	
	CCorutinePlus* GetCorutine();
	void ReleaseCorutine(CCorutinePlus* pPTR);

	int GetVTCorutineSize(){ return m_vtCorutinePlus.size(); }
	int GetCreateCorutineTimes(){return m_usCreateTimes;}
protected:
	CCorutinePlus* CreateCorutine(bool bPush);
protected:
#ifdef USE_UCONTEXT
	ucontext_t              			m_ctxMain;
#else
	coctx_t								m_ctxMain;
#endif
	char 								m_stack[STACK_SIZE];
	typedef basiclib::basic_vector<CCorutinePlus*>::type	VTCorutinePlus;
	VTCorutinePlus						m_vtCorutinePlus;
	unsigned short						m_usCreateTimes;
	int									m_nDefaultStackSize;

	friend class CCorutinePlus;
};


#endif
