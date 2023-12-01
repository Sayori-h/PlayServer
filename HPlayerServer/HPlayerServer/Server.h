#pragma once
#include "Socket.h"
#include "CEpoll.h"
#include "ThreadPool.h"
#include "Process.h"
#include "Logger.h"

class CBusiness
{
public:
	virtual int BusinessProcess() = 0;

	template<typename Function,typename...Args>
	int setConnected(Function func, Args...args) {
		m_connected = new CFunction(func, args...);
		if (m_connected == nullptr)return -1;
		return 0;
	}

	template<typename Function,typename...Args>
	int setRecvDone(Function func, Args...args) {
		m_recvdone = new CFunction(func, args...);
		if (m_recvdone == nullptr)return -1;
		return 0;
	}
private:
	CFunction* m_connected;
	CFunction* m_recvdone;
};

class CServer
{
public:
	CServer();
	~CServer();
	CServer(const CServer&) = delete;
	CServer operator=(const CServer&) = delete;

	int Init(CBusiness* business,const Buffer& ip = "127.0.0.1", short port = 9999); 		
	int Run();
	int Close();
private:
	int ThreadFunc();
private:
	CThreadPool m_pool;
	CSocketBase* m_server;
	CEpoll m_epoll;
	CProcess m_process;
	//业务模块 需要我们手动delete
	CBusiness* m_business;
};

