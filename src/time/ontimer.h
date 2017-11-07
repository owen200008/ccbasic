#ifndef BASIC_ONTIMER_H
#define BASIC_ONTIMER_H

//Timing-Wheel 
//define the ontimer, timeout, 

#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////////////////
typedef void(*pOnTimerCallback)(intptr_t, intptr_t pParam1);
///////////////////////////////////////////////////////////////////////////////////////////
#define TIME_NEAR_SHIFT 8
#define TIME_NEAR (1 << TIME_NEAR_SHIFT)
#define TIME_LEVEL_SHIFT 6
#define TIME_LEVEL (1 << TIME_LEVEL_SHIFT)
#define TIME_NEAR_MASK (TIME_NEAR-1)
#define TIME_LEVEL_MASK (TIME_LEVEL-1)

struct timer_event 
{
	pOnTimerCallback	m_callbackFunc;
	intptr_t			m_nKey;
	intptr_t			m_pParam1;
};

struct timer_node 
{
	timer_node*			next;
	uint32_t 			expire;
	uint32_t			m_nRepeatTime;
	uint8_t				m_bIsValid;
};

struct link_list 
{
	timer_node	head;
	timer_node*	tail;
};
typedef basiclib::basic_map<intptr_t, timer_node*>	 MapTimerNode;
///////////////////////////////////////////////////////////////////////////////////////////
#pragma warning (push)
#pragma warning (disable: 4251)
#pragma warning (disable: 4275)
class _BASIC_DLL_API CBasicOnTimer : public basiclib::CBasicObject
{
public:
	CBasicOnTimer();
	virtual ~CBasicOnTimer();

	bool InitTimer();
	void CloseTimer();
	void WaitThreadExit();

	/*	最高精度10ms, 实际误差25ms(sleep)
		100=1s 
		实际100ms精度内的无法完全准确
	*/
	bool AddTimeOut(intptr_t nKey, pOnTimerCallback pFunc, int nTimes, intptr_t pParam1);
	bool AddOnTimer(intptr_t nKey, pOnTimerCallback pFunc, int nTimes, intptr_t pParam1);
	void DelTimer(intptr_t nKey);
protected:
	friend THREAD_RETURN Timer_Thread(void* pParam);
	void TimerRun();

	void TimerUpdate();
protected:
	void timer_add(timer_event& event, int time, int bRepeat);
	void timer_del(intptr_t nKey);

	timer_node* link_clear(link_list *list);
	void timer_execute(basiclib::CSpinLockFuncNoSameThreadSafe& lock);
	void timer_shift();
	void move_list(int level, int idx);
	void dispatch_list(struct timer_node *current);
	void add_node(timer_node *node);
	void linklist(link_list *list, timer_node *node);
protected:
	bool							m_bTimerExit;
	HANDLE							m_hThread;

	link_list                		m_near[TIME_NEAR];
	link_list                		m_t[4][TIME_LEVEL];
	basiclib::SpinLock              m_lock;
	uint32_t                        m_time;
	uint32_t                        starttime;
	uint64_t                       	m_current;
	uint64_t                        m_current_point;
	MapTimerNode*					m_pMapNode;
};
#pragma warning (pop)

#endif