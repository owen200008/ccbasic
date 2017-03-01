#include "../inc/basic.h"
/*
CBasicPoll::CBasicPoll()
{

}
CBasicPoll::~CBasicPoll()
{

}

void BasicPollReadSelfOrder(int fd, short event, void *arg)
{
	CBasicPoll *thr = (CBasicPoll*)arg;

	CBasicPollNotify eventQueue;
	int nReadLength = 0;
	do{
		nReadLength = recv(thr->m_pair[0], (char*)&eventQueue, sizeof(CBasicPollNotify), 0);
		if (nReadLength == sizeof(CBasicPollNotify)){
			eventQueue.m_func(eventQueue.m_ud);
		}
		else if (nReadLength < 0){
			break;
		}
	} while (nReadLength != sizeof(CBasicPollNotify));
}

bool CBasicPoll::Init(event_config *cfg)
{
	bool bReleaseCFG = false;
	if (nullptr == cfg)
		cfg = event_config_new();
	if (evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, m_pair) == -1)
	{
		return false;
	}
	m_base = event_base_new_with_config(cfg);
	event_set(&notify_event, m_pair[0], EV_READ | EV_PERSIST, BasicPollReadSelfOrder, this);
	event_base_set(m_base, &notify_event);
	if (event_add(&notify_event, NULL) == -1){
		return false;
	}
	if (bReleaseCFG){
		event_config_free(cfg);
	}
	return true;
	}*/