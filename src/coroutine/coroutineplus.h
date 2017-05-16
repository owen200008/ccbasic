#ifndef INC_COROUTINEPLUS_H
#define INC_COROUTINEPLUS_H

typedef void(*coctx_pfn_t)(const char* s);
struct coctx_t
{
#if defined(__x86_64__)
    void *regs[12];
#else
	void *regs[6];
#endif

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

#define STACK_SIZE 			(256*1024)
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

    template<class T>
    T GetYieldParam(int nParam)
    {
        if (nParam < 0 || nParam >= RESUME_MAXPARAM)
            nParam = 0;
        void* pPTR = m_pResumeParam[nParam];
        return *((T*)pPTR);
    }
    template<class T>
    T* GetYieldParamPoint(int nParam)
    {
        if (nParam < 0 || nParam >= RESUME_MAXPARAM)
            nParam = 0;
        void* pPTR = m_pResumeParam[nParam];
        return (T*)pPTR;
    }
    template<class T1>
    void YieldCorutine(T1& param1){
        m_pResumeParam[0] = &param1;
        YieldCorutine();
    }
    template<class T1, class T2>
    void YieldCorutine(T1& param1, T2& param2){
        m_pResumeParam[0] = &param1;
        m_pResumeParam[1] = &param2;
        YieldCorutine();
    }
    template<class T1, class T2, class T3>
    void YieldCorutine(T1& param1, T2& param2, T3& param3)
    {
        m_pResumeParam[0] = &param1;
        m_pResumeParam[1] = &param2;
        m_pResumeParam[2] = &param3;
        YieldCorutine();
    }
    template<class T1, class T2, class T3, class T4>
    void YieldCorutine(T1& param1, T2& param2, T3& param3, T4& param4)
    {
        m_pResumeParam[0] = &param1;
        m_pResumeParam[1] = &param2;
        m_pResumeParam[2] = &param3;
        m_pResumeParam[3] = &param4;
        YieldCorutine();
    }
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

    CoroutineState Resume(CCorutinePlusPool* pPool);
	template<class T1>
    CoroutineState Resume(CCorutinePlusPool* pPool, T1& param1)
	{
		m_pResumeParam[0] = &param1;
		return Resume(pPool);
	}
	template<class T1, class T2>
    CoroutineState Resume(CCorutinePlusPool* pPool, T1& param1, T2& param2)
	{
		m_pResumeParam[0] = &param1;
		m_pResumeParam[1] = &param2;
		return Resume(pPool);
	}
	template<class T1, class T2, class T3>
    CoroutineState Resume(CCorutinePlusPool* pPool, T1& param1, T2& param2, T3& param3)
	{
		m_pResumeParam[0] = &param1;
		m_pResumeParam[1] = &param2;
		m_pResumeParam[2] = &param3;
		return Resume(pPool);
	}
	template<class T1, class T2, class T3, class T4>
    CoroutineState Resume(CCorutinePlusPool* pPool, T1& param1, T2& param2, T3& param3, T4& param4)
	{
		m_pResumeParam[0] = &param1;
		m_pResumeParam[1] = &param2;
		m_pResumeParam[2] = &param3;
		m_pResumeParam[3] = &param4;
		return Resume(pPool);
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

    //! 判断是否是死循环或者没有唤醒
    bool IsCoroutineError(time_t tmNow);
protected:
    void InitStackAndPool(CCorutinePlusPool* pPool, char* pStack);
protected:
	CoroutineState							m_state;
	coroutine_func 							m_func;
	coctx_t									m_ctx;
    time_t                                  m_tmResumeTime;

	//resume param
	CCorutinePlusPool*						m_pRunPool;
	void*									m_pResumeParam[RESUME_MAXPARAM];
    CCorutinePlusPool*                      m_pCreatePool;

	//是否hook sys io函数,默认是true
	bool									m_bSysHook;

    friend class CCorutinePlusPool;
    friend class CCorutinePlusPoolMgr;
};


//动态平衡
class CCorutinePlusPoolBalance : public basiclib::CBasicObject
{
protected:
    CCorutinePlusPoolBalance();
    virtual ~CCorutinePlusPoolBalance();

    //! 获取多余的corutine
    int GetCorutineMore(CCorutinePlus* pPTR[], int nCount);
    //! 多余的Corutine放这边
    void ReleaseCorutineMore(CCorutinePlus* pPTR[], int nCount);

    //! 获取堆栈
    int GetExtraStack(char* pBuf[], int nCount);
    void ReleaseExtraStack(char* pBuf[], int nCount);

    int GetExtraCorutineCount();
    int GetExtraShareStackCount();
protected:
    basiclib::CMessageQueueLock<char*>              m_queueExtraStack;
    basiclib::CMessageQueueLock<CCorutinePlus*>     m_queueExtraCorutine;
    friend class CCorutinePlusPool;
};

//协程超时管理
class CCorutinePlusPoolMgr : public basiclib::CMessageQueueLock<CCorutinePlus*>
{
protected:
    CCorutinePlusPoolMgr();
    virtual ~CCorutinePlusPoolMgr();

    void CreateCorutine(CCorutinePlus* pPTR);

    //! 检查所有的协程是否有问题
    void CheckAllCorutine();
protected:
    friend class CCorutinePlusPool;
};

//thread not safe
#define CorutinePlus_Max_Stack	32
class _BASIC_DLL_API CCorutinePlusPool : public basiclib::CBasicObject
{
public:
	CCorutinePlusPool();
	virtual ~CCorutinePlusPool();

    bool InitCorutine(int nDefaultSize = DEFAULT_COROUTINE, int nLimitCorutineCount = 1024 * 2, uint16_t nShareStackSize = 10, uint16_t nMaxCreateShareStackSize = 1024);
	
	CCorutinePlus* GetCorutine(bool bGlobal = true);

    uint32_t GetVTCorutineSize(){ return m_nCorutineSize; }
    uint32_t GetCreateCorutineTimes(){ return m_nCreateTimes; }

    uint32_t GetVTShareStackCount(){ return m_usShareStackSize; }
    uint32_t GetCreateTimesShareStackCount(){ return m_nCreateTimesShareStack; }

    //异常的协程需要回收
    void ReleaseCorutine(CCorutinePlus* pPTR);
protected:
    void ResumeFunc(CCorutinePlus* pNext);
    void YieldFunc(CCorutinePlus* pCorutine);
    void FinishFunc(CCorutinePlus* pCorutine);

    char* GetShareStack(bool bGlobal = true);
    void ReleaseShareStack(char* pStack);
protected:
	CCorutinePlus*  CreateCorutine();
    char*           CreateShareStack();
private:
    bool                                m_bInit;
    typedef basiclib::basic_vector<char*>	VTSTACKS;
    VTSTACKS                            m_vtStacks;
    uint16_t                            m_usShareStackSize;
    uint16_t                            m_usRealShareStackSize;
    uint32_t                            m_nCreateTimesShareStack;
    uint16_t                            m_usMaxCreateShareStackSize;

	typedef basiclib::basic_vector<CCorutinePlus*>	VTCorutinePlus;
	VTCorutinePlus						m_vtCorutinePlus;
	uint32_t						    m_nCreateTimes;
    uint32_t                            m_nCorutineSize;
    uint32_t                            m_nRealVTCorutineSize;
    uint32_t                            m_nLimitSize;           //如果超过这个数字，协程会放到公共里面去

	CCorutinePlus*						m_pStackRunCorutine[CorutinePlus_Max_Stack];
	unsigned short						m_usRunCorutineStack;
	CCorutinePlus						m_selfPlus;

    static CCorutinePlusPoolBalance     m_balance;
    static CCorutinePlusPoolMgr         m_poolMgr;

    friend class CCorutinePlus;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class _BASIC_DLL_API CCorutinePlusThreadData : public basiclib::CBasicObject
{
public:
    //初始化在callback里面做,返回param，必须是new出来的
    CCorutinePlusThreadData(basiclib::CBasicThreadTLS* pTLS, const std::function<void*(CCorutinePlusThreadData*)>& callback, const std::function<void(void*)>& releaseFunc);
    virtual ~CCorutinePlusThreadData();

    DWORD GetThreadID(){ return m_dwThreadID; }
    CCorutinePlusPool* GetCorutinePlusPool(){ return &m_pool; }
protected:
    CCorutinePlusPool									    m_pool;
    DWORD												    m_dwThreadID;
    void*									                m_pParam;
    std::function<void(void*)>                              m_releaseFunc;
};

#pragma warning (pop)


#endif
