#ifndef INC_COROUTINEPLUS_H
#define INC_COROUTINEPLUS_H

typedef void(*coctx_pfn_t)(const char* s);
struct coctx_t
{
#ifdef __BASICWINDOWS
    void *regs[6];
#else
#if defined(__i386__)
    void *regs[6];
#elif defined(__x86_64__)
    void *regs[13];
#endif
#endif

    size_t ss_size;
    char *ss_sp;
    coctx_t(){
        memset(this, 0, sizeof(coctx_t));
    }
};

enum CoroutineState
{
    CoroutineState_Death    = 0,
	CoroutineState_Ready 	= 1,
	CoroutineState_Running	= 2,
	CoroutineState_Suspend	= 3,
};

#define STACK_SIZE 			(512*1024)
#define DEFAULT_COROUTINE 	16
#define RESUME_MAXPARAM		4
class CCorutinePlus;
class CCorutinePlusPool;
typedef void (*coroutine_func)(CCorutinePlus* pCorutinePlus);

#pragma warning (push)
#pragma warning (disable: 4251)
#pragma warning (disable: 4275)
class _BASIC_DLL_API CCorutinePlus : public basiclib::CBasicObject
{
public:
	CCorutinePlus();
	virtual ~CCorutinePlus();
	
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
	CoroutineState GetCoroutineState(){ return m_state; }
	//no call self
	void StartFunc();
	void StartFuncLibco();

	//设置是否hook io函数，目前只有unix linux有效（参考libco）
	void SetSysHook(bool bSysHook){
		m_bSysHook = bSysHook;
	}
	bool IsSysHook(){
		return m_bSysHook;
	}
protected:
    void CheckStackSize(int nDefaultStackSize);
    void SaveStack();
    void ResumeStack();
protected:
	CoroutineState							m_state;
	coroutine_func 							m_func;
	coctx_t									m_ctx;

	char*									m_stack;
	int										m_nCap;
	int										m_nSize;

    //使用的pool stack
    char*                                   m_pUseRunPoolStack;
	//resume param
	CCorutinePlusPool*						m_pRunPool;
	void*									m_pResumeParam[RESUME_MAXPARAM];

	//是否hook sys io函数,默认是true
	bool									m_bSysHook;

    friend class CCorutinePlusPool;
};

//thread not safe
#define CorutinePlus_Max_Stack	32
class _BASIC_DLL_API CCorutinePlusPool : public basiclib::CBasicObject
{
public:
	CCorutinePlusPool();
	virtual ~CCorutinePlusPool();

	bool InitCorutine(int nDefaultSize = DEFAULT_COROUTINE, int nDefaultStackSize = 1024 * 16, int nShareStackSize = 2);
	
	CCorutinePlus* GetCorutine();

	int GetVTCorutineSize(){ return m_vtCorutinePlus.size(); }
    uint32_t GetCreateCorutineTimes(){ return m_nCreateTimes; }
    uint16_t GetStackCreateTimes(){ return m_usStackSize; }
protected:
    void ResumeFunc(CCorutinePlus* pNext);
    void YieldFunc(CCorutinePlus* pCorutine);
    void FinishFunc(CCorutinePlus* pCorutine);
protected:
	CCorutinePlus* CreateCorutine();
	void ReleaseCorutine(CCorutinePlus* pPTR);

    char* GetStack(int nIndex);
private:
    typedef basiclib::basic_vector<char*>	VTSTACKS;
    VTSTACKS                            m_vtStacks;
    uint16_t                            m_usStackSize;

	typedef basiclib::basic_vector<CCorutinePlus*>	VTCorutinePlus;
	VTCorutinePlus						m_vtCorutinePlus;
	uint32_t						    m_nCreateTimes;
    uint32_t                            m_nCorutineSize;
    uint32_t                            m_nRealVTCorutineSize;
    
	int									m_nDefaultStackSize;

	CCorutinePlus*						m_pStackRunCorutine[CorutinePlus_Max_Stack];
	unsigned short						m_usRunCorutineStack;
	CCorutinePlus						m_selfPlus;

    friend class CCorutinePlus;
};



#pragma warning (pop)


#endif
