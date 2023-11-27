#include "Thread.h"
std::map<pthread_t, CThread*> CThread::m_mapThread;

CThread::CThread()
{
	m_function = nullptr;
	m_thread = 0;
	m_bPaused = false;
}

int CThread::Start()
{
	int ret = 0;
	pthread_attr_t attr;
	ret=pthread_attr_init(&attr);
	if (ret)return -1;
	//1.不需要主线程join
	ret=pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	if (ret)return -2;
	//2.设置线程竞争范围为进程内部,默认的
	//ret=pthread_attr_setscope(&attr, PTHREAD_SCOPE_PROCESS);
	//if (ret)return -3;
	ret=pthread_create(&m_thread, &attr, CThread::ThreadEntry, this);
	if (ret)return -3;
	m_mapThread[m_thread] = this;
	ret=pthread_attr_destroy(&attr);
	if (ret)return -4;
	return 0;
}

int CThread::Pause()
{
	if (m_thread != 0)return -1;
	if (m_bPaused) {
		m_bPaused = false;
		return 0;
	}
	m_bPaused = true;
	int ret=pthread_kill(m_thread, SIGUSR1);
	if (ret != 0) {
		m_bPaused = false;
		return -2;
	}
	return 0;
}

int CThread::Stop()
{
	if (m_thread != 0) {
		pthread_t temp_thread = m_thread;
		m_thread = 0;
		timespec ts;
		ts.tv_sec = 0;
		ts.tv_nsec = 100 * (int)1e6;//100ms
		int ret=pthread_timedjoin_np(temp_thread, nullptr, &ts);
		if (ret == ETIMEDOUT) {
			pthread_detach(temp_thread);
			pthread_kill(temp_thread, SIGUSR2);
		}
	}
	return 0;
}

bool CThread::isVaild() const
{
	return m_thread!=0;
}

void* CThread::ThreadEntry(void* arg)
{
	CThread* thiz = (CThread*)arg;

	struct sigaction act = { 0 };
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = &CThread::Sigaction;
	sigaction(SIGUSR1, &act, NULL);
	sigaction(SIGUSR2, &act, NULL);

	thiz->EnterThread();
	//不是冗余，有可能被stop函数把m_thread给清零了
	if (thiz->m_thread)thiz->m_thread = 0;
	pthread_t thread = pthread_self();
	auto it = m_mapThread.find(thread);
	if (it != m_mapThread.end())m_mapThread[thread] = nullptr;
	pthread_detach(thread);

	pthread_exit(nullptr);
}

void CThread::Sigaction(int signo, siginfo_t* info, void* context)
{
	if (signo == SIGUSR1) {//pause sig
		pthread_t thread = pthread_self();
		auto it = m_mapThread.find(thread);
		if (it != m_mapThread.end())
			if (it->second != nullptr)
				while (it->second->m_bPaused == true) {
					if (it->second->m_thread == 0)
						pthread_exit(nullptr);
					usleep(1000);//1ms
				}					
	}
	else if (signo == SIGUSR2) {//stop sig
		pthread_exit(nullptr);
	}
}

void CThread::EnterThread()
{
	if (m_function != nullptr) {
		int ret = (*m_function)();
		if (ret != 0) {
#ifdef _DEBUG
			{
				std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
				memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
				snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> ret=%d\n", 
					__FILE__, __LINE__, __FUNCTION__, ret);
				fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
				fflush(pFile);
			}
#endif // DEBUG  
		}
	}
}
