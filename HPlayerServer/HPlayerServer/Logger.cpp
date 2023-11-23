#include "Logger.h"

CLoggerServer::CLoggerServer():
	m_thread(&CLoggerServer::ThreadFunc,this)
{
	m_server = nullptr;
	m_path = "./log/" + GetTimeStr() + ".log";
#ifdef _DEBUG
	snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> path=%s\n", __FILE__, __LINE__, __FUNCTION__, m_path);
	fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
	fflush(pFile);
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
	ret = m_thread.Start();
	if (ret != 0) {
		Close();
		return -6;
	}
	return 0;
}

int CLoggerServer::ThreadFunc()
{
	std::vector<epoll_event> events;
	std::map<int, CSocketBase*> mapClients;
	while (m_thread.isVaild()&& (m_epoll != -1) && (m_server != nullptr)) {
		ssize_t ret = m_epoll.WaitEvents(events, 1);
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
						if (r < 0)continue;
						r = m_epoll.Add(*pClient, EpollData((void*)pClient), EPOLLIN | EPOLLERR);
						if (r < 0) {
							delete pClient;
							continue;
						}
						auto it = mapClients.find(*pClient);
						if (it->second != nullptr)delete it->second;
						mapClients[*pClient] = pClient;
					}
					else {
						CSocketBase* pClient = (CSocketBase*)events[i].data.ptr;
						if (pClient != nullptr) {
							Buffer data(1024 * 1024);
							int r = pClient->Recv(data);
							if (r <= 0) {
								delete pClient;
								mapClients[*pClient] = nullptr;
							}
							else WriteLog(data);
						}
					}
				}
			}
			if (i != ret)break;
		}
	}
	for (auto it = mapClients.begin(); it !=mapClients.end(); it++) {
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
	static thread_local CLocalSocket client;
	if (client == -1) {
		int ret = 0;
		ret = client.Init(CSockParam("./log/server.sock", 0));
		if (ret != 0) {
#ifdef _DEBUG
			snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);
			fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
			fflush(pFile);
#endif // DEBUG 
			return;
		}
	}
	client.Send(info);
}

Buffer CLoggerServer::GetTimeStr()
{
	Buffer result(128);
	timeb tmb;
	ftime(&tmb);
	tm* pTm = localtime(&tmb.time);
	int nSize = snprintf(result,result.size(),
		"%04d-%02d-%02d+%02d-%02d-%02d+%03d",
		pTm->tm_year + 1900, pTm->tm_mon +1, pTm->tm_mday,
		pTm->tm_hour, pTm->tm_min, pTm -> tm_sec,tmb.millitm);
	result.resize(nSize);
	return result;
}

void CLoggerServer::WriteLog(const Buffer& data)
{
	if (m_file != nullptr) {
		FILE* pfile = m_file;
		fwrite(data, 1, data.size(), pfile);
		fflush(pfile);
#ifdef _DEBUG
		snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> log=%s\n", __FILE__, __LINE__, __FUNCTION__, data);
		fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
		fflush(pFile);
#endif // DEBUG 
	}
}
