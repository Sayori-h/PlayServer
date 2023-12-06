#pragma once
#include "Socket.h"
#include "CEpoll.h"
#include "ThreadPool.h"
#include "Process.h"
#include "Logger.h"

class CConnectedFunction {
public:
	template<typename Function, typename...Args>
	CConnectedFunction(Function func, Args...args) {
		m_binder = std::bind(func, args...);
	}

	~CConnectedFunction() {}

	int operator()(CSocketBase* pClient) {
		return m_binder(pClient);
	}
private:
	std::function<int(CSocketBase*)> m_binder;
};

class CReceivedFunction {
public:
	template<typename Function, typename...Args>
	CReceivedFunction(Function func, Args...args) {
		m_binder = std::bind(func, args...);
	}

	~CReceivedFunction() {}

	int operator()(CSocketBase* pClient, const Buffer& data) {
		return m_binder(pClient,data);
	}
private:
	std::function<int(CSocketBase*, const Buffer&)> m_binder;
};

class CBusiness
{
public:
	CBusiness();

	virtual int BusinessProcess(CProcess* proc) = 0;

	template<typename Function,typename...Args>
	int setConnected(Function func, Args...args) {
		m_connected = new CConnectedFunction(func, args...);
		if (m_connected == nullptr)return -1;
		return 0;
	}

	template<typename Function,typename...Args>
	int setRecvDone(Function func, Args...args) {
		m_recvdone = new CReceivedFunction(func, args...);
		if (m_recvdone == nullptr)return -1;
		return 0;
	}
protected:
	CConnectedFunction* m_connected;
	CReceivedFunction* m_recvdone;
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

