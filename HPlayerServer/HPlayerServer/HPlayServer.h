#pragma once
#include "Server.h"

#define ERR_RETURN(ret, err) \
if(ret!=0){\
	TRACE_ERROR("ret= %d errno = %d msg = [%s]\n", ret, errno, strerror(errno));\
	return err;}

#define WARN_CONTINUE(ret) \
if(ret!=0){\
	TRACE_WARNING("ret= %d errno = %d msg = [%s]\n", ret, errno, strerror(errno));\
	continue;}

class CHPlayServer:public CBusiness	
{
public:
	CHPlayServer(unsigned count);
	~CHPlayServer();
	//业务模块主线程
	virtual int BusinessProcess(CProcess* proc) override;
private:
	//客户端处理线程
	int ThreadFunc();
private:
	CEpoll m_epoll;
	std::map<int, CSocketBase*>m_mapClients;
	CThreadPool m_pool;
	unsigned m_count;
};

