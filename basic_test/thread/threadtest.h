#ifndef INC_THREADTEST_H
#define INC_THREADTEST_H

#include <basic.h>

using namespace basiclib;

CBasicThreadTLS tls;
int* g_p = 0;
void SetGI(int* pN){
	g_p = pN;
}
int* GetGI(){
	return g_p;
}

void ReadSelfOrder(int fd, short event, void *arg)
{
}

THREAD_RETURN WorkerThreadTest(void *arg)
{
	void* pRet = BasicGetBasiclibGlobalTLS(0);
	printf("%d\n", pRet == nullptr ? 0 : 1);
	pRet = BasicGetBasiclibGlobalTLS(1);
	printf("%d\n", pRet == nullptr ? 0 : 1);
#define TIMES_FORTEST 1000000
		/*{
				clock_t begin = clock();
				int i = 0;
				for (i = 0; i < TIMES_FORTEST; i++){
					int n = i;
					SetGI(&n);
					int* pRet = GetGI();
				}
				clock_t end = clock();
				printf("int设置读取效率 %d:%d\n", i, end - begin);
		}
		{
				clock_t begin = clock();
				int i = 0;
				for (i = 0; i < TIMES_FORTEST; i++){
					int n = i;
					tls.SetValue(&n);
					int* pRet = (int*)tls.GetValue();
				}

				clock_t end = clock();
				printf("tls存取效率 %d:%d\n", i, end - begin);
		}

				{
					CEvent event(TRUE, FALSE, nullptr);
					LONG m_lWaitThreadCount = 0;
					clock_t begin = clock();
					for (int i = 0; i < TIMES_FORTEST; i++){
	
					}

					clock_t end = clock();
					printf("event %d:%d\n", TIMES_FORTEST, end - begin);
				}
		{
			CBasicSessionNetClient::CreateClient(0);

			struct event_config *cfg = event_config_new();
			struct event_base*	m_base = event_base_new_with_config(cfg);
			evutil_socket_t		m_pair[2];
			if (evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, m_pair) == -1)
			{
				basiclib::BasicLogEventError("create evutil_socketpair error");
				return 0;
			}
			clock_t begin = clock();
			for (int i = 0; i < 100000; i++){
				struct event		notify_event;
				event_set(&notify_event, m_pair[0], EV_READ | EV_PERSIST, ReadSelfOrder, nullptr);
				event_base_set(m_base, &notify_event);
				if (event_add(&notify_event, NULL) == -1)
				{
					basiclib::BasicLogEventErrorV("libevent eventadd error %d", i);
					break;
				}
				//send(m_pair[1], (const char*)"1", 1, 0);
				event_del(&notify_event);
			}

			clock_t end = clock();
			printf("notify %d:%d\n", TIMES_FORTEST, end - begin);
			evutil_closesocket(m_pair[0]);
			evutil_closesocket(m_pair[1]);
			event_config_free(cfg);
			event_base_free(m_base);
		}*/
		{
			CBasicSessionNetClient::CreateClient(0);

			struct event_config *cfg = event_config_new();
			struct event_base*	m_base = event_base_new_with_config(cfg);
			evutil_socket_t		m_pair[2];
			if (evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, m_pair) == -1)
			{
				basiclib::BasicLogEventError("create evutil_socketpair error");
				return 0;
			}
			char buf[16] = { 0 };
			int nTotal = 0;
			int nRead = 0;
			clock_t begin = clock();
			
			int i = 0;
			for (i = 0; i < 100000; i++){
				nTotal += send(m_pair[1], (const char*)"1", 1, 0);
				nRead += recv(m_pair[0], buf, 1, 0);
			}

			clock_t end = clock();
			printf("notifysend %d:%d %d %d\n", i, end - begin, nTotal, nRead);
			evutil_closesocket(m_pair[0]);
			evutil_closesocket(m_pair[1]);
			event_config_free(cfg);
			event_base_free(m_base);
		}
		

	return 0;
}


void TestThread()
{
	void* pRet = BasicGetBasiclibGlobalTLS(0);
	printf("%d\n", pRet == nullptr ? 0 : 1);
	pRet = BasicGetBasiclibGlobalTLS(1);
	printf("%d\n", pRet == nullptr ? 0 : 1);
	BasicSetBasiclibGlobalTLS(0, malloc(1));
	pRet = BasicGetBasiclibGlobalTLS(0);
	printf("%d\n", pRet == nullptr ? 0 : 1);
	pRet = BasicGetBasiclibGlobalTLS(1);
	printf("%d\n", pRet == nullptr ? 0 : 1);
	tls.CreateTLS();
	for (int i = 0; i < 4; i++)
	{
		DWORD dwThreadServerID = 0;
		basiclib::BasicCreateThread(WorkerThreadTest, nullptr, &dwThreadServerID);
	}
}


#endif