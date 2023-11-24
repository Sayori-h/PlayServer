#pragma once
#include "Thread.h"
#include "CEpoll.h"
#include "Socket.h"
#include "Debug.h"
#include <sys/timeb.h>
#include <stdarg.h>
#include <sstream>
#include <sys/stat.h>

enum LogLevel {
	LOG_INFO,
	LOG_DEBUG,
	LOG_WARNING,
	LOG_ERROR,
	LOG_FATAL
};

class LogInfo
{
public:
	LogInfo(const char* file, int line, const char* func, pid_t pid,
		pthread_t tid,int logType,const char*format,...);
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
		m_buf += stream.str();
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

#ifndef TRACE
#define TRACE_INFO(...) CLoggerServer::Trace(LogInfo(__FILE__,__LINE__,\
__FUNCTION__,getpid(),pthread_self(),LOG_INFO,__VA_ARGS__))
#define TRACE_DEBUG(...) CLoggerServer::Trace(LogInfo(__FILE__,__LINE__,\
__FUNCTION__,getpid(),pthread_self(),LOG_DEBUG,__VA_ARGS__))
#define TRACE_WARNING(...) CLoggerServer::Trace(LogInfo(__FILE__,__LINE__,\
__FUNCTION__,getpid(),pthread_self(),LOG_WARNING,__VA_ARGS__))
#define TRACE_ERROR(...) CLoggerServer::Trace(LogInfo(__FILE__,__LINE__,\
__FUNCTION__,getpid(),pthread_self(),LOG_ERROR,__VA_ARGS__))
#define TRACE_FATAL(...) CLoggerServer::Trace(LogInfo(__FILE__,__LINE__,\
__FUNCTION__,getpid(),pthread_self(),LOG_FATAL,__VA_ARGS__))

//LOG_INFO<<"hello"<<"how are you";只有流式日志是自己主动发的
#define LOG_INFO LogInfo(__FILE__,__LINE__,__FUNCTION__,getpid(),pthread_self(),LOG_INFO)
#define LOG_DEBUG LogInfo(__FILE__,__LINE__,__FUNCTION__,getpid(),pthread_self(),LOG_DEBUG)
#define LOG_WARNING LogInfo(__FILE__,__LINE__,__FUNCTION__,getpid(),pthread_self(),LOG_WARNING)
#define LOG_ERROR LogInfo(__FILE__,__LINE__,__FUNCTION__,getpid(),pthread_self(),LOG_ERROR)
#define LOG_FATAL LogInfo(__FILE__,__LINE__,__FUNCTION__,getpid(),pthread_self(),LOG_FATAL)

//内存导出
//00 01 02 65…… ; ...a……
#define DUMP_INFO(data,size) LogInfo(__FILE__,__LINE__,__FUNCTION__,getpid(),\
pthread_self(),LOG_INFO,data,size)
#define DUMP_DEBUG(data,size) LogInfo(__FILE__,__LINE__,__FUNCTION__,getpid(),\
pthread_self(),LOG_DEBUG,data,size)
#define DUMP_WARNING(data,size) LogInfo(__FILE__,__LINE__,__FUNCTION__,getpid(),\
pthread_self(),LOG_WARNING,data,size)
#define DUMP_ERROR(data,size) LogInfo(__FILE__,__LINE__,__FUNCTION__,getpid(),\
pthread_self(),LOG_ERROR,data,size)
#define DUMP_FATAL(data,size) LogInfo(__FILE__,__LINE__,__FUNCTION__,getpid(),\
pthread_self(),LOG_FATAL,data,size)
#endif

