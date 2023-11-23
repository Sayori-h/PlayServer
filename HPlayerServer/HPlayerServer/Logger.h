#pragma once
#include "Thread.h"
#include "CEpoll.h"
#include "Socket.h"
#include "Debug.h"
#include <sys/timeb.h>
#include <sys/stat.h>

class LogInfo
{
public:
	LogInfo();
	~LogInfo();
	operator Buffer()const { return m_buf; }
private:
	Buffer m_buf;
};

class CLoggerServer
{
public:
	CLoggerServer();
	~CLoggerServer();

	CLoggerServer(const CLoggerServer&) = delete;
	CLoggerServer& operator=(const CLoggerServer&) = delete;

	int Start();
	int ThreadFunc();
	int Close();
	//给其他非日志进程和线程用的
	static void Trace(const LogInfo& info);
	static Buffer GetTimeStr();
private:
	void WriteLog(const Buffer& data);
private:
	CThread m_thread;
	CEpoll m_epoll;
	CSocketBase* m_server;
	Buffer m_path;
	FILE* m_file;
};

