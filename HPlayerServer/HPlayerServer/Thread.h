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

	int Start();//��ʼ/�ָ�
	int Pause();//��ͣ
	int Stop();//ֹͣ
	bool isVaild()const;//�����߳�(��һ�����б������)
private:
	//��ֹ����
	CThread(const CThread&) = delete;
	CThread operator=(const CThread&) = delete;

	static void* ThreadEntry(void* arg);//__stdcall
	static void Sigaction(int signo, siginfo_t* info, void* context);//�����ź���
	void EnterThread();//__thiscall
private:
	CFunction* m_function;
	pthread_t m_thread;
	bool m_bPaused;//true ��ʾ��ͣ false��ʾ������
	static std::map<pthread_t, CThread*>m_mapThread;
};

