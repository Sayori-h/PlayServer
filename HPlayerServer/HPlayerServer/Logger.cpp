#include "Logger.h"

CLoggerServer::CLoggerServer() :
	m_thread(&CLoggerServer::ThreadFunc, this)
{
	m_server = nullptr;
	char curPath[256] = "";
	getcwd(curPath, sizeof(curPath));
	m_path = curPath;
	m_path += "/log/" + GetTimeStr() + ".log";
#ifdef _DEBUG
	{
		std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
		memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
		snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> path=%s\n", __FILE__, 
			__LINE__, __FUNCTION__, (char*)m_path);
		fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
		fflush(pFile);
	}
#endif // DEBUG 
}

CLoggerServer::~CLoggerServer()
{
	Close();
}

int CLoggerServer::Start()
{
	if (m_server != nullptr)return -1;
	if (access("log", W_OK | R_OK) != 0)
		mkdir("log", S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	m_file = fopen(m_path, "w+");
	if (m_file == NULL)return -2;
	int ret = m_epoll.Create(1);
	if (ret != 0)return -3;
	m_server = new CLocalSocket();
	if (m_server == nullptr) {
		Close();
		return -4;
	}
	ret = m_server->Init(CSockParam("./log/server.sock", (int)SOCK_ISSERVER));
	if (ret != 0) {
		Close();
		return -5;
	}
	ret=m_epoll.Add(*m_server, EpollData((void*)m_server), EPOLLIN | EPOLLERR);
	if (ret != 0) {
		Close();
		return -6;
	}
	ret = m_thread.Start();
	if (ret != 0) {
#ifdef _DEBUG
		{
			std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
			memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
			snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> pid=%d errno:%d msg:%s ret:%d\n",
				__FILE__, __LINE__, __FUNCTION__, getpid(), errno, strerror(errno), ret);
			fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
			fflush(pFile);
		}
#endif
		Close();
		return -7;
	}
	return 0;
}

int CLoggerServer::ThreadFunc()
{
#ifdef _DEBUG
	{
		std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
		memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
		snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> %d %d %p\n", __FILE__,
			__LINE__, __FUNCTION__, m_thread.isVaild(), (int)m_epoll, m_server);
		fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
		fflush(pFile);
	}
#endif
	std::vector<epoll_event> events;
	std::map<int, CSocketBase*> mapClients;
	while (m_thread.isVaild() && (m_epoll != -1) && (m_server != nullptr)) {
		ssize_t ret = m_epoll.WaitEvents(events, 1000);
#ifdef _DEBUG
	{
		std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
		memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
		snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> ret=%d\n",
			__FILE__, __LINE__, __FUNCTION__, ret);
		fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
		fflush(pFile);
	}
#endif
		if (ret < 0)break;
		if (ret > 0) {
			ssize_t i = 0;
			for (; i < ret; i++) {
				if (events[i].events & EPOLLERR) {//有错误
					break;
				}
				else if (events[i].events & EPOLLIN) {//套接字上有数据可读
					if (events[i].data.ptr == m_server) {
						CSocketBase* pClient = nullptr;
						int r = m_server->Link(&pClient);
#ifdef _DEBUG
						{
							std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
							memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
							snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> res=%d\n",
								__FILE__, __LINE__, __FUNCTION__, r);
							fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
							fflush(pFile);
						}
#endif
						if (r < 0)continue;
						r = m_epoll.Add(*pClient, EpollData((void*)pClient), EPOLLIN | EPOLLERR);
#ifdef _DEBUG
						{
							std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
							memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
							snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> res=%d\n",
								__FILE__, __LINE__, __FUNCTION__, r);
							fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
							fflush(pFile);
						}
#endif
						if (r < 0) {
							delete pClient;
							continue;
						}
						auto it = mapClients.find(*pClient);
						if (it != mapClients.end()) {
							if (it->second != nullptr)delete it->second;
						}
						mapClients[*pClient] = pClient;
#ifdef _DEBUG
						{
							std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
							memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
							snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s>\n",
								__FILE__, __LINE__, __FUNCTION__);
							fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
							fflush(pFile);
						}
#endif
					}
					else {
#ifdef _DEBUG
						{
							std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
							memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
							snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> ptr=%p\n",
								__FILE__, __LINE__, __FUNCTION__, events[i].data.ptr);
							fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
							fflush(pFile);
						}
#endif
						CSocketBase* pClient = (CSocketBase*)events[i].data.ptr;
						if (pClient != nullptr) {
							Buffer data(1024 * 1024);
							int r = pClient->Recv(data);
#ifdef _DEBUG
							{
								std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
								memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
								snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> res=%d\n",
									__FILE__, __LINE__, __FUNCTION__, r);
								snprintf(szBufInfo + strlen(szBufInfo), BUF_SIZE - strlen(szBufInfo),
									"errno:%d msg:%s\n", errno, strerror(errno));
								fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
								fflush(pFile);
							}
#endif
							if (r <= 0) {
								mapClients[*pClient] = nullptr;
								delete pClient;
#ifdef _DEBUG
								{
									std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
									memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
									snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> res=%d\n",
										__FILE__, __LINE__, __FUNCTION__, r);
									fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
									fflush(pFile);
								}
#endif
							}
							else {
#ifdef _DEBUG
								{
									std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
									memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
									snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> data=%s\n",
										__FILE__, __LINE__, __FUNCTION__, (char*)data);
									fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
									fflush(pFile);
								}
#endif
								WriteLog(data);
#ifdef _DEBUG
								{
									std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
									memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
									snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s>\n",
										__FILE__, __LINE__, __FUNCTION__);
									fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
									fflush(pFile);
								}
#endif
							}
						}
					}
				}
			}
			if (i != ret)break;
		}
	}
	for (auto it = mapClients.begin(); it != mapClients.end(); it++) {
		if (it->second) delete it->second;
	}
	mapClients.clear();
	return 0;
}

int CLoggerServer::Close()
{
	if (m_server != nullptr) {
		CSocketBase* temp = m_server;
		m_server = nullptr;
		delete temp;
	}
	m_epoll.Close();
	m_thread.Stop();
	return 0;
}

void CLoggerServer::Trace(const LogInfo& info)
{
	int ret = 0;
	static thread_local CLocalSocket client;
	if (client == -1) {
		ret = client.Init(CSockParam("./log/server.sock", 0));
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
			return;
		}
#ifdef _DEBUG
		{
			std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
			memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
			snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> ret=%d client=%d\n",
				__FILE__, __LINE__, __FUNCTION__, ret, (int)client);
			fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
			fflush(pFile);
		}
#endif // DEBUG 
		ret = client.Link();
#ifdef _DEBUG
		{
			std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
			memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
			snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> ret=%d client=%d\n",
				__FILE__, __LINE__, __FUNCTION__, ret, (int)client);
			fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
			fflush(pFile);
		}
#endif // DEBUG 
	}
	ret=client.Send(info);
#ifdef _DEBUG
	{
		std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
		memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
		snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> ret=%d client=%d\n",
			__FILE__, __LINE__, __FUNCTION__, ret, (int)client);
		fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
		fflush(pFile);
	}
#endif // DEBUG 
}

Buffer CLoggerServer::GetTimeStr()
{
	Buffer result(128);
	timeb tmb;
	ftime(&tmb);
	tm* pTm = localtime(&tmb.time);
	int nSize = snprintf(result, result.size(),
		"%04d-%02d-%02d_%02d-%02d-%02d.%03d",
		pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday,
		pTm->tm_hour, pTm->tm_min, pTm->tm_sec, tmb.millitm);
	result.resize(nSize);
	return result;
}

void CLoggerServer::WriteLog(const Buffer& data)
{
	if (m_file != nullptr) {
		FILE* pfile = m_file;
		fwrite((char*)data, 1, data.size(), pfile);
		fflush(pfile);
#ifdef _DEBUG
		{
			std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
			memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
			snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> log=%s\n", __FILE__, __LINE__, __FUNCTION__, (char*)data);
			fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
			fflush(pFile);
		}
#endif // DEBUG 
	}
}

LogInfo::LogInfo(const char* file, int line, const char* func, pid_t pid,
	pthread_t tid, int logType, const char* format, ...)
{
	const char sLogType[][8] = { "INFO","DEBUG","WARNING","ERROR","FATAL" };
	char* buf = nullptr;
	m_bIsStreamLog = false;
	int count = asprintf(&buf, "%s(%d):[%s][%s]<%d-%d>(%s) ", file, line, sLogType[logType],
		(char*)CLoggerServer::GetTimeStr(), pid, tid, func);
	if (count > 0) {
		m_buf = buf;
		free(buf);
	}
	else return;

	va_list ap;
	va_start(ap, format);
	count = vasprintf(&buf, format, ap);
	if (count > 0) {
		m_buf += buf;
		free(buf);
	}
	m_buf += '\n';
	va_end(ap);
}

LogInfo::LogInfo(const char* file, int line, const char* func,
	pid_t pid, pthread_t tid, int logType)
{
	char* buf = nullptr;
	m_bIsStreamLog = true;//自己主动发送的 流式日志	
	const char sLogType[][8] = { "INFO","DEBUG","WARNING","ERROR","FATAL" };
	int count = asprintf(&buf, "%s(%d):[%s][%s]<%d-%d>(%s) ", file, line, sLogType[logType],
		(char*)CLoggerServer::GetTimeStr(), pid, tid, func);
	if (count > 0) {
		m_buf = buf;
		free(buf);
	}
}
/*测试用例：
const char* message = "Hello, Logging!";
LogInfo log("example.cpp", 42, "main", getpid(), pthread_self(), LOG_DEBUG, message, strlen(message));

example.cpp(42):[DEBUG][时间]<进程ID-线程ID>(main)
48 65 6C 6C 6F 2C 20 4C 6F 67 67 69 6E 67 21 00    ; Hello, Logging!.

核心是通过 snprintf 将每个字节以两位的十六进制格式追加到字符串 m_buf 中。在每行的末尾，将可读的字符表示追加到字符串中。
如果数据总长度不是16的倍数，最后一行会进行特殊处理，确保对齐和显示。
*/
LogInfo::LogInfo(const char* file, int line, const char* func,
	pid_t pid, pthread_t tid, int logType, void* pData, size_t nSize)
{
	char* buf = nullptr;
	m_bIsStreamLog = false;
	const char sLogType[][8] = { "INFO","DEBUG","WARNING","ERROR","FATAL" };
	int count = asprintf(&buf, "%s(%d):[%s][%s]<%d-%d>(%s)\n", file, line, sLogType[logType],
		(char*)CLoggerServer::GetTimeStr(), pid, tid, func);
	if (count > 0) {
		m_buf = buf;
		free(buf);
	}
	else return;

	size_t i = 0;
	char* Data = (char*)pData;
	for (; i < nSize; i++) {
		char buf[16] = "";
		snprintf(buf, sizeof(buf), "%02X ", Data[i] & 0xFF);
		m_buf += buf;
		if (0 == ((i + 1) % 16)) {
			m_buf += "\t; ";
			char buf[17] = "";
			memcpy(buf, Data + i - 15, 16);
			for (int j = 0; j < 16; j++)
				if (buf[j] < 32&&(buf[j]>=0))buf[j] = '.';
			m_buf += buf;
			/*for (size_t j = i - 15; j <= i; j++) {
				
				if ((Data[j] & 0xFF) > 31 && ((Data[j] & 0xFF) < 0x7f))
					m_buf += Data[i];
				else m_buf += '.';
			}*/
			m_buf += '\n';
		}
	}
	//处理最后一行
	size_t k = i % 16;
	if (k != 0) {
		for (size_t j = 0; j < 16 - k; j++) m_buf += "   ";
		m_buf += "\t; ";
		for (size_t j = i - k; j <= i; j++) {
			if ((Data[i] & 0xFF) > 31 && ((Data[j] & 0xFF) < 0x7F))
				m_buf += Data[i];
			else m_buf += '.';
		}
		m_buf += "\n";
	}	
}

LogInfo::~LogInfo()
{
	if (m_bIsStreamLog) {
		m_buf += '\n';
		CLoggerServer::Trace(*this);
	}
}
