#include "HPlayServer.h"

CHPlayServer::CHPlayServer(unsigned count)
	:CBusiness()
{
	m_count = count;
}

CHPlayServer::~CHPlayServer()
{
	m_epoll.Close();
	m_pool.Close();
	for (auto it : m_mapClients)
		if (it.second != nullptr) {
			delete it.second;
		}
	m_mapClients.clear();
}

int CHPlayServer::BusinessProcess(CProcess* proc)
{
	int ret = 0;
	ret = m_epoll.Create(m_count);
	ERR_RETURN(ret, -1);
	ret = m_pool.Start(m_count);
	ERR_RETURN(ret, -2);
	for (unsigned i = 0; i < m_count; i++) {
		ret = m_pool.AddTask(&CHPlayServer::ThreadFunc, this);
		ERR_RETURN(ret, -3);
	}
	int sock = 0;
	sockaddr_in addrin;
	while (m_epoll != -1) {//检测状态
		ret = proc->RecvSocket(sock,&addrin);
		if (ret < 0 || (sock == 0))break;
		CSocketBase* pClient = new CSocket(sock);
		if (pClient == nullptr)continue;
		ret = pClient->Init(CSockParam(&addrin,SOCK_ISIP));
		WARN_CONTINUE(ret);
		ret = m_epoll.Add(sock, (EpollData)(void*)pClient);
		if (m_connected) {
			(*m_connected)();
		}
		WARN_CONTINUE(ret);
	}
	return 0;
}

int CHPlayServer::ThreadFunc()
{
	int ret = 0;
	std::vector<epoll_event> events;
	while ((m_epoll != -1)) {
		ssize_t size = m_epoll.WaitEvents(events);
		if (size < 0)break;
		if (size > 0) {
			for (ssize_t i = 0; i < size; i++) {
				if (events[i].events & EPOLLERR)break;
				else if (events[i].events & EPOLLIN) {//处理读取事件
					CSocketBase* pClient = (CSocketBase*)events[i].data.ptr;
					if (pClient != nullptr) {
						Buffer data;
						ret = pClient->Recv(data);
						WARN_CONTINUE(ret);
						if (m_recvdone != nullptr)
							(*m_recvdone)();
					}					
				}
			}
		}
	}
	return 0;
}
