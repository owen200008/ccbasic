#ifndef INC_ONTIMERTEST_H
#define INC_ONTIMERTEST_H

#include <basic.h>

CBasicOnTimer timer;
void TimeOut100MS()
{
	TRACE("TimeOut100MS %d\n", clock());
	timer.AddTimeOut(TimeOut100MS, 100);
}

void TimeOut2S()
{
	TRACE("TimeOut2S %d\n", clock());
	timer.AddTimeOut(TimeOut2S, 2000);
}

void TimeOnTimer200MS()
{
	static int i = 0;
	TRACE("OnTimer200MS %d\n", clock());
	i++;
	if (i == 100)
	{
		timer.DelTimer(TimeOnTimer200MS);
	}
}

void TimeOnTimer1S()
{
	static int i = 0;
	TRACE("TimeOnTimer1S %d\n", clock());
	i++;
	if (i == 10)
	{
		timer.DelTimer(TimeOnTimer1S);
	}
}

void OnTimerTest(){
	TimeOut100MS();
	TimeOut2S();
	TimeOnTimer1S();
}

#endif