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
class CCorutinePlusBase;
class CCorutinePlusPoolBase;
typedef void (*coroutine_func)(CCorutinePlusBase* pCorutinePlus);

#pragma warning (push)
#pragma warning (disable: 4251)
#pragma warning (disable: 4275)
//! 修改过一个版本使用tuple实现参数传递，不过必须C++14，另外效率上和当前版本效率差距比较大，因此参数采用赋值的形式
class _BASIC_DLL_API CCorutinePlusBase : public basiclib::CBasicObject
{
public:
	CCorutinePlusBase();
	virtual ~CCorutinePlusBase();

	//! 初始化 coroutine_func
	void Create(coroutine_func pFunc);

	//! 唤醒
	template<class...T>
	CoroutineState Resume(CCorutinePlusPoolBase* pPool, T...args){
		void* pResumeParam[sizeof...(args)] = { args... };
		m_pResumeParam = &pResumeParam;
		return Resume(pPool);
	}

	//! 睡觉
	template<class...T>
	void YieldCorutine(T...args){
		void* pResumeParam[sizeof...(args)] = { args... };
		m_pResumeParam = &pResumeParam;
		YieldCorutine();
	}

	template<class T>
	T GetParam(int nParam){
		void* pPTR = ((void**)m_pResumeParam)[nParam];
		return *((T*)pPTR);
	}
	template<class T>
	T* GetParamPoint(int nParam){
		void* pPTR = ((void**)m_pResumeParam)[nParam];
		return (T*)pPTR;
	}
	///////////////////////////////////////////////////////////////////////////////////
	//! 唤醒
	CoroutineState Resume(CCorutinePlusPoolBase* pPool);

	//! 睡觉
	void YieldCorutine();
	///////////////////////////////////////////////////////////////////////////////////
	
	CCorutinePlusPoolBase* GetRunPool(){return m_pRunPool;}
	CoroutineState GetCoroutineState(){ return m_state; }
	//no call self
	virtual void StartFuncLibco();

    //! 判断是否是死循环或者没有唤醒
    bool IsCoroutineError(time_t tmNow);
protected:
    void InitStackAndPool(CCorutinePlusPoolBase* pPool, char* pStack);
protected:
	CoroutineState							m_state;
	coroutine_func 							m_func;
	coctx_t									m_ctx;
    time_t                                  m_tmResumeTime;

	//resume param
	CCorutinePlusPoolBase*					m_pRunPool;
	void*									m_pResumeParam;
	CCorutinePlusPoolBase*                  m_pCreatePool;

    friend class CCorutinePlusPoolBase;
    friend class CCorutinePlusPoolMgr;
};


//动态平衡
class CCorutinePlusPoolBalance : public basiclib::CBasicObject
{
protected:
    CCorutinePlusPoolBalance();
    virtual ~CCorutinePlusPoolBalance();

    //! 获取多余的corutine
    int GetCorutineMore(CCorutinePlusBase* pPTR[], int nCount);
    //! 多余的Corutine放这边
    void ReleaseCorutineMore(CCorutinePlusBase* pPTR[], int nCount);

    //! 获取堆栈
    int GetExtraStack(char* pBuf[], int nCount);
    void ReleaseExtraStack(char* pBuf[], int nCount);

    int GetExtraCorutineCount();
    int GetExtraShareStackCount();

	//! 获取创建的堆栈
	char* GetNewCreateStack(int nTimes, int& nCreateSize);
protected:
    basiclib::CMessageQueueLock<char*>              m_queueExtraStack;
    basiclib::CMessageQueueLock<CCorutinePlusBase*>     m_queueExtraCorutine;
	basiclib::CMessageQueueLock<char*>              m_queueCreateStack;
    friend class CCorutinePlusPoolBase;
};

//协程超时管理
class CCorutinePlusPoolMgr : public basiclib::CMessageQueueLock<CCorutinePlusBase*>
{
protected:
    CCorutinePlusPoolMgr();
    virtual ~CCorutinePlusPoolMgr();

    void CreateCorutine(CCorutinePlusBase* pPTR);

    //! 检查所有的协程是否有问题
    void CheckAllCorutine();
protected:
    friend class CCorutinePlusPoolBase;
};

//thread not safe
#define CorutinePlus_Max_Stack	32
class _BASIC_DLL_API CCorutinePlusPoolBase : public basiclib::CBasicObject
{
public:
	CCorutinePlusPoolBase();
	virtual ~CCorutinePlusPoolBase();

    bool InitCorutine(int nDefaultSize = DEFAULT_COROUTINE, int nLimitCorutineCount = 1024 * 2, uint16_t nShareStackSize = 10, uint16_t nMaxCreateShareStackSize = 1024);
	
	CCorutinePlusBase* GetCorutine(bool bGlobal = true);

    uint32_t GetVTCorutineSize(){ return m_nCorutineSize; }
    uint32_t GetCreateCorutineTimes(){ return m_nCreateTimes; }

    uint32_t GetVTShareStackCount(){ return m_usShareStackSize; }
    uint32_t GetCreateTimesShareStackCount(){ return m_nCreateTimesShareStack; }

    //异常的协程需要回收
    void ReleaseCorutine(CCorutinePlusBase* pPTR);
protected:
    void ResumeFunc(CCorutinePlusBase* pNext);
    void YieldFunc(CCorutinePlusBase* pCorutine);
    virtual void FinishFunc(CCorutinePlusBase* pCorutine);

    char* GetShareStack(bool bGlobal = true);
    void ReleaseShareStack(char* pStack);
protected:
	CCorutinePlusBase*  CreateCorutine();
    char*				CreateShareStack();

	virtual CCorutinePlusBase* ConstructCorutine(){ return new CCorutinePlusBase(); }
private:
    bool                                m_bInit;
    typedef basiclib::basic_vector<char*>	VTSTACKS;
    basiclib::CMessageQueue<char*>      m_vtCreateStack;
    VTSTACKS                            m_vtStacks;
    uint16_t                            m_usShareStackSize;
    uint16_t                            m_usRealShareStackSize;
    uint32_t                            m_nCreateTimesShareStack;
    uint16_t                            m_usMaxCreateShareStackSize;

	typedef basiclib::basic_vector<CCorutinePlusBase*>	VTCorutinePlus;
	VTCorutinePlus						m_vtCorutinePlus;
	uint32_t						    m_nCreateTimes;
    uint32_t                            m_nCorutineSize;
    uint32_t                            m_nRealVTCorutineSize;
    uint32_t                            m_nLimitSize;           //如果超过这个数字，协程会放到公共里面去

	CCorutinePlusBase*					m_pStackRunCorutine[CorutinePlus_Max_Stack];
	unsigned short						m_usRunCorutineStack;
	CCorutinePlusBase					m_selfPlus;

    static CCorutinePlusPoolBalance     m_balance;
    static CCorutinePlusPoolMgr         m_poolMgr;

    friend class CCorutinePlusBase;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class _BASIC_DLL_API CCorutinePlusThreadDataBase : public basiclib::CBasicObject
{
public:
    //初始化在callback里面做,返回param，必须是new出来的
	CCorutinePlusThreadDataBase();
    virtual ~CCorutinePlusThreadDataBase();

	void Init();

    DWORD GetThreadID(){ return m_dwThreadID; }
	CCorutinePlusPoolBase* GetCorutinePlusPool(){ return m_pPool; }
protected:
	virtual CCorutinePlusPoolBase* CreatePool(){ return new CCorutinePlusPoolBase(); }
protected:
    CCorutinePlusPoolBase*									m_pPool;
    DWORD												    m_dwThreadID;
};

#pragma warning (pop)


#endif
