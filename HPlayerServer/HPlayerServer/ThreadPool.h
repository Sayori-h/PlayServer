#pragma once
#include "Socket.h"
#include "CEpoll.h"
#include "Thread.h"
#include "Function.h"

class CThreadPool
{
public:
	CThreadPool();
	~CThreadPool();
	CThreadPool(const CThreadPool&) = delete;
	CThreadPool& operator=(const CThreadPool&) = delete;		
public:
	int Start(unsigned count);
	void Close();
	template<typename _FUNCTION_, typename..._ARGS_>
	int AddTask(_FUNCTION_ func, _ARGS_... args) {
		static thread_local CSocket client;
		int ret = 0;
		if (client == -1) {
			ret = client.Init(CSockParam(m_path, 0));
			if (ret != 0)return -1;
			ret = client.Link();
			if (ret != 0)return -2;
		}
		CFunction* base = new CFunction(func, args...);
		if (base == nullptr)return -3;
		Buffer data(sizeof(base));
		memcpy(data, &base, sizeof(base));
		ret = client.Send(data);
		if (ret != 0) {
			delete base;
			return -4;
		}
		return 0;
	}
private:
	int TaskDispatch();
private:
	CEpoll m_epoll;
	std::vector<CThread*> m_threads;
	CSocketBase* m_server;
	Buffer m_path;
};

