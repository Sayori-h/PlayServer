#pragma once
#include "Thread.h"
#include "CEpoll.h"
#include "Socket.h"
#include <sys/timeb.h>
#include <stdarg.h>
#include <sstream>
#include <sys/stat.h>

enum LogType {
	INFO,
	DEBUG,
	WARNING,
	ERROR,
	FATAL
};

class LogInfo
{
public:
	LogInfo(const char* file, int line, const char* func, pid_t pid,
		pthread_t tid, int logType,const char*format,...);
	LogInfo(const char* file, int line, const char* func, pid_t pid,
		pthread_t tid, int logType);
	LogInfo(const char* file, int line, const char* func, pid_t pid,
		pthread_t tid, int logType,void*pData,size_t nSize);
	~LogInfo();
	operator Buffer()const { return m_buf; }
	template<typename T>
	LogInfo& operator<<(const T& data) {
		std::stringstream stream;
		stream << data;
#ifdef DEBUG
		{
			std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
			memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
			snprintf(szBufInfo, BUF_SIZE, "%s(%d):[%s] stringstream=[%s]\n",
				__FILE__, __LINE__, __FUNCTION__, stream.str().c_str());
			fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
			fflush(pFile);
		}
#endif
		m_buf += stream.str().c_str();
#ifdef DEBUG
		{
			std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
			memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
			snprintf(szBufInfo, BUF_SIZE, "%s(%d):[%s] m_buf=[%s]\n",
				__FILE__, __LINE__, __FUNCTION__, (char*)m_buf);
			fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
			fflush(pFile);
		}
#endif
		return *this;
	}
private:
	Buffer m_buf;
	//默认是false 流式日志，则为true
	bool m_bIsStreamLog;
};

class CLoggerServer
{
public:
	CLoggerServer();
	~CLoggerServer();

	CLoggerServer(const CLoggerServer&) = delete;
	CLoggerServer& operator=(const CLoggerServer&) = delete;

	int Start();
	int Close();
	//给其他非日志进程和线程用的
	static void Trace(const LogInfo& info);
	static Buffer GetTimeStr();
private:
	int ThreadFunc();
	void WriteLog(const Buffer& data);
private:
	CThread m_thread;
	CEpoll m_epoll;
	CSocketBase* m_server;
	Buffer m_path;
	FILE* m_file;
};

#ifndef TRACE
#define TRACE_INFO(...) CLoggerServer::Trace(LogInfo(__FILE__,__LINE__,__FUNCTION__,\
getpid(),pthread_self(),INFO,__VA_ARGS__))
#define TRACE_DEBUG(...) CLoggerServer::Trace(LogInfo(__FILE__,__LINE__,__FUNCTION__,\
getpid(),pthread_self(),DEBUG,__VA_ARGS__))
#define TRACE_WARNING(...) CLoggerServer::Trace(LogInfo(__FILE__,__LINE__,__FUNCTION__,\
getpid(),pthread_self(),WARNING,__VA_ARGS__))
#define TRACE_ERROR(...) CLoggerServer::Trace(LogInfo(__FILE__,__LINE__,__FUNCTION__,\
getpid(),pthread_self(),ERROR,__VA_ARGS__))
#define TRACE_FATAL(...) CLoggerServer::Trace(LogInfo(__FILE__,__LINE__,__FUNCTION__,\
getpid(),pthread_self(),FATAL,__VA_ARGS__))

//LOG_INFO<<"hello"<<"how are you";只有流式日志是自己主动发的
#define LOG_INFO LogInfo(__FILE__,__LINE__,__FUNCTION__,getpid(),pthread_self(),INFO)
#define LOG_DEBUG LogInfo(__FILE__,__LINE__,__FUNCTION__,getpid(),pthread_self(),DEBUG)
#define LOG_WARNING LogInfo(__FILE__,__LINE__,__FUNCTION__,getpid(),pthread_self(),WARNING)
#define LOG_ERROR LogInfo(__FILE__,__LINE__,__FUNCTION__,getpid(),pthread_self(),ERROR)
#define LOG_FATAL LogInfo(__FILE__,__LINE__,__FUNCTION__,getpid(),pthread_self(),FATAL)

//内存导出
//00 01 02 65…… ; ...a……
#define DUMP_INFO(data,size) CLoggerServer::Trace(LogInfo(__FILE__,__LINE__,__FUNCTION__,getpid(),\
pthread_self(),INFO,data,size))
#define DUMP_DEBUG(data,size) CLoggerServer::Trace(LogInfo(__FILE__,__LINE__,__FUNCTION__,getpid(),\
pthread_self(),DEBUG,data,size))
#define DUMP_WARNING(data,size) CLoggerServer::Trace(LogInfo(__FILE__,__LINE__,__FUNCTION__,getpid(),\
pthread_self(),WARNING,data,size))
#define DUMP_ERROR(data,size) CLoggerServer::Trace(LogInfo(__FILE__,__LINE__,__FUNCTION__,getpid(),\
pthread_self(),ERROR,data,size))
#define DUMP_FATAL(data,size) CLoggerServer::Trace(LogInfo(__FILE__,__LINE__,__FUNCTION__,getpid(),\
pthread_self(),FATAL,data,size))
#endif

