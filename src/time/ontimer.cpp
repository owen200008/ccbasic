#include "../inc/basic.h"

typedef void(*timer_execute_func)(void *ud, void *arg);

#ifdef WIN32
int gettimeofday(struct timeval *tp, void *tzp)
{
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;

	GetLocalTime(&wtm);
	tm.tm_year = wtm.wYear - 1900;
	tm.tm_mon = wtm.wMonth - 1;
	tm.tm_mday = wtm.wDay;
	tm.tm_hour = wtm.wHour;
	tm.tm_min = wtm.wMinute;
	tm.tm_sec = wtm.wSecond;
	tm.tm_isdst = -1;
	clock = mktime(&tm);
	tp->tv_sec = clock;
	tp->tv_usec = wtm.wMilliseconds * 1000;

	return (0);
}
#endif

// centisecond: 1/100 second
static void systime(uint32_t *sec, uint32_t *cs)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	*sec = tv.tv_sec;
	*cs = tv.tv_usec / 10000;
}

static uint64_t gettime()
{
	uint64_t t;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	t = (uint64_t)tv.tv_sec * 100;
	t += tv.tv_usec / 10000;
	return t;
}
/////////////////////////////////////////////////////////////////////////////////
CBasicOnTimer::CBasicOnTimer()
{
	m_bTimerExit = false;

	memset(m_near, 0, sizeof(link_list) * TIME_NEAR);
	memset(m_t, 0, sizeof(link_list) * TIME_LEVEL * 4);
	m_time = 0;
	starttime = 0;
	m_current = 0;
	m_current_point = 0;

	m_pMapNode = new MapTimerNode();
	m_hThread = nullptr;
}

CBasicOnTimer::~CBasicOnTimer()
{
	if (!m_bTimerExit)
	{
		CloseTimer();
	}
	if (m_pMapNode)
	{
		delete m_pMapNode;
		m_pMapNode = nullptr;
	}
}

THREAD_RETURN Timer_Thread(void* pParam)
{
	CBasicOnTimer* pServer = (CBasicOnTimer*)pParam;
	pServer->TimerRun();
	return 0;
}

void CBasicOnTimer::TimerRun()
{
	while (!m_bTimerExit)
	{
		TimerUpdate();
#ifdef __BASICWINDOWS
		basiclib::BasicSleep(25);
#else
		usleep(2500);
#endif
	}
}

void CBasicOnTimer::TimerUpdate()
{
	uint64_t cp = gettime();
	if (cp < m_current_point)
	{
		basiclib::BasicLogEventErrorV("time diff error: change from %lld to %lld", cp, m_current_point);
		m_current_point = cp;
	}
	else if (cp != m_current_point)
	{
		uint32_t diff = (uint32_t)(cp - m_current_point);
		m_current_point = cp;
		m_current += diff;
		for (int i = 0; i<diff; i++)
		{
			basiclib::CSpinLockFuncNoSameThreadSafe lock(&m_lock, TRUE);

			// try to dispatch timeout 0 (rare condition)
			timer_execute(lock);

			// shift time first, and then dispatch timer message
			timer_shift();

			timer_execute(lock);
		}
	}
}

void CBasicOnTimer::timer_execute(basiclib::CSpinLockFuncNoSameThreadSafe& lock)
{
	int idx = m_time & TIME_NEAR_MASK;

	while (m_near[idx].head.next)
	{
		timer_node *current = link_clear(&m_near[idx]);

		lock.UnLock();
		// dispatch_list don't need lock T
		dispatch_list(current);
		lock.Lock();
	}
}

void CBasicOnTimer::timer_shift()
{
	int mask = TIME_NEAR;
	uint32_t ct = ++m_time;
	if (ct == 0)
	{
		move_list(3, 0);
	}
	else
	{
		uint32_t time = ct >> TIME_NEAR_SHIFT;
		int i = 0;

		while ((ct & (mask - 1)) == 0)
		{
			int idx = time & TIME_LEVEL_MASK;
			if (idx != 0)
			{
				move_list(i, idx);
				break;
			}
			mask <<= TIME_LEVEL_SHIFT;
			time >>= TIME_LEVEL_SHIFT;
			++i;
		}
	}
}

void CBasicOnTimer::move_list(int level, int idx)
{
	timer_node *current = link_clear(&m_t[level][idx]);
	while (current)
	{
		timer_node *temp = current->next;
		add_node(current);
		current = temp;
	}
}

void CBasicOnTimer::dispatch_list(struct timer_node *current)
{
	do
	{
		timer_event * event = (struct timer_event *)(current + 1);
		if (current->m_bIsValid)
		{
			event->m_callbackFunc(event->m_nKey, event->m_pParam1);
		}
		timer_node* temp = current;
		current = current->next;
		if (temp->m_nRepeatTime > 0 && temp->m_bIsValid)
		{
			//spin unlock, so add is valid
			temp->expire += temp->m_nRepeatTime;

			basiclib::CSpinLockFuncNoSameThreadSafe lock(&m_lock, TRUE);
			add_node(temp);
		}
		else
		{
			basiclib::BasicDeallocate(temp);
		}
	} while (current);
}

void CBasicOnTimer::timer_add(timer_event& event, int time, int bRepeat)
{
	int sz = sizeof(event);
	struct timer_node *node = (struct timer_node *)basiclib::BasicAllocate(sizeof(*node) + sz);
	memcpy(node + 1, &event, sz);
	node->m_bIsValid = 1;
	node->m_nRepeatTime = bRepeat > 0 ? time : 0;
	basiclib::CSpinLockFunc lock(&m_lock, TRUE);
	node->expire = time + m_time;
	if (bRepeat > 0)
	{
		(*m_pMapNode)[event.m_nKey] = node;
	}
	add_node(node);
}

void CBasicOnTimer::timer_del(Net_PtrInt nKey)
{
	basiclib::CSpinLockFunc lock(&m_lock, TRUE);
	MapTimerNode::iterator iter = (*m_pMapNode).find(nKey);
	if (iter != (*m_pMapNode).end())
	{
		timer_node* pNode = iter->second;
		pNode->m_bIsValid = 0;
		(*m_pMapNode).erase(iter);
	}
}

void CBasicOnTimer::add_node(timer_node *node)
{
	uint32_t time = node->expire;
	uint32_t current_time = m_time;

	if ((time | TIME_NEAR_MASK) == (current_time | TIME_NEAR_MASK))
	{
		linklist(&m_near[time&TIME_NEAR_MASK], node);
	}
	else
	{
		int i;
		uint32_t mask = TIME_NEAR << TIME_LEVEL_SHIFT;
		for (i = 0; i<3; i++)
		{
			if ((time | (mask - 1)) == (current_time | (mask - 1)))
			{
				break;
			}
			mask <<= TIME_LEVEL_SHIFT;
		}

		linklist(&m_t[i][((time >> (TIME_NEAR_SHIFT + i*TIME_LEVEL_SHIFT)) & TIME_LEVEL_MASK)], node);
	}
}

void CBasicOnTimer::linklist(link_list *list, timer_node *node)
{
	list->tail->next = node;
	list->tail = node;
	node->next = 0;
}

bool CBasicOnTimer::InitTimer()
{
	if (m_hThread)
		return false;
	int i, j;
	for (i = 0; i < TIME_NEAR; i++)
	{
		link_clear(&m_near[i]);
	}
	for (i = 0; i<4; i++)
	{
		for (int j = 0; j < TIME_LEVEL; j++)
		{
			link_clear(&m_t[i][j]);
		}
	}
	uint32_t current = 0;
	systime(&starttime, &current);
	m_current = current;
	m_current_point = gettime();

	DWORD nThreadId = 0;
	m_hThread = basiclib::BasicCreateThread(Timer_Thread, this, &nThreadId);
	return true;
}

void CBasicOnTimer::WaitThreadExit()
{
	if (m_hThread)
		basiclib::BasicWaitThread(m_hThread);
}

void CBasicOnTimer::CloseTimer()
{
	m_bTimerExit = true;
}

bool CBasicOnTimer::AddTimeOut(Net_PtrInt nKey, pOnTimerCallback pFunc, int nTimes, Net_PtrInt pParam1)
{
	if (nTimes <= 0)
	{
		pFunc(nKey, pParam1);
	}
	else
	{
		struct timer_event event;
		event.m_callbackFunc = pFunc;
		event.m_nKey = nKey;
		event.m_pParam1 = pParam1;
		timer_add(event, nTimes, 0);
	}
	return true;
}

bool CBasicOnTimer::AddOnTimer(Net_PtrInt nKey, pOnTimerCallback pFunc, int nTimes, Net_PtrInt pParam1)
{
	if (time > 0)
	{
		struct timer_event event;
		event.m_callbackFunc = pFunc;
		event.m_nKey = nKey;
		event.m_pParam1 = pParam1;
		timer_add(event, nTimes, 1);
		return true;
	}
	return false;
}

void CBasicOnTimer::DelTimer(Net_PtrInt nKey)
{
	timer_del(nKey);
}

timer_node* CBasicOnTimer::link_clear(link_list *list)
{
	timer_node * ret = list->head.next;
	list->head.next = 0;
	list->tail = &(list->head);

	return ret;
}