/***********************************************************************************************
// 文件名:     basicpoll.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2017/2/9 12:00:18
// 内容描述:   事件驱动
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_BASICPOLL_H
#define BASIC_BASICPOLL_H

#include "event.h"

typedef void(*pCallBasicPollNotify)(void* m_ud);
struct CBasicPollNotify
{
	pCallBasicPollNotify							m_func;
	void*											m_ud;
};

//基于libevent实现,底层如果不是fd驱动的会卡死
class CBasicPoll : public basiclib::CBasicObject
{
public:
	CBasicPoll();
	virtual ~CBasicPoll();

	/* 初始化线程basicpool
	*/
	bool Init(event_config *cfg);
protected:
	friend void BasicPollReadSelfOrder(int fd, short event, void *arg);
protected:
	evutil_socket_t		m_pair[2];
	struct event_base*	m_base;
	struct event		notify_event;
};

#endif