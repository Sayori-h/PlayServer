#pragma once
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>
#include <map>
#include <errno.h>
#include "Function.h"
#include "Debug.h"

class CThread
{
public:
	CThread();

	template<typename Function, typename...Args>
	CThread(Function func, Args... args)
		:m_function(new CFunction(func, args...)), 
		m_thread(0), m_bPaused(false) {}

	~CThread() {}

	template<typename Function, typename...Args>
	int SetThreadFunc(Function func, Args... args)
	{
		m_function = new CFunction(func, args...);
		if (m_function == nullptr)return -1;
		return 0;
	}

	int Start();//开始/恢复
	int Pause();//暂停
	int Stop();//停止
	bool isVaild()const;
private:
	//禁止复制
	CThread(const CThread&) = delete;
	CThread operator=(const CThread&) = delete;

	static void* ThreadEntry(void* arg);//__stdcall
	static void Sigaction(int signo, siginfo_t* info, void* context);//处理信号量
	void EnterThread();//__thiscall
private:
	CFunction* m_function;
	pthread_t m_thread;
	bool m_bPaused;//true 表示暂停 false表示运行中
	static std::map<pthread_t, CThread*>m_mapThread;
};

