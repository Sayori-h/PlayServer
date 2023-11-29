#include "ThreadPool.h"


CThreadPool::CThreadPool()
{
	m_server = nullptr;
	timespec tp{ 0,0 };
	clock_gettime(CLOCK_REALTIME, &tp);
	char* buf = nullptr;
	asprintf(&buf, "%d.%d.sock", tp.tv_sec % (int)1e5, tp.tv_nsec % (int)1e6);
	if (buf != nullptr) {
		m_path = buf;
		free(buf);
	}//如果path初始化失败，在Start里处理
	struct timespec sleep_time {}, remaining{};	
	sleep_time.tv_nsec = 100;// 设置休眠时间为 100 ns
	nanosleep(&sleep_time, &remaining);
}

CThreadPool::~CThreadPool()
{
	Close();
}

int CThreadPool::Start(unsigned count)
{
	int ret = 0;
	if (m_server != nullptr)return -1;//已经初始化
	if (m_path.empty())return -2;//构造函数失败
	m_server = new CLocalSocket();
	if (m_server == nullptr)return -3;//内存分配失败
	ret = m_server->Init(CSockParam(m_path, SOCK_ISSERVER));
	if (ret != 0)return -4;
	ret = m_epoll.Create(count);
	if (ret != 0)return -5;
	ret = m_epoll.Add(*m_server, static_cast<EpollData>((void*)m_server));
	if (ret != 0)return -6;
	for (unsigned i = 0; i < m_threads.size(); i++) {
		m_threads[i] = new CThread(&CThreadPool::TaskDispatch, this);
		if (m_threads[i] == nullptr)return -7;
		ret = m_threads[i]->Start();
		if (ret != 0)return - 8;
	}
	return 0;
}

void CThreadPool::Close()
{
	m_epoll.Close();
	if (m_server != nullptr) {
		CSocketBase* pTemp = m_server;
		m_server = nullptr;
		delete pTemp;
	}
	for (auto thread : m_threads) {
		if (thread != nullptr)delete thread;
	}
	m_threads.clear();
	unlink(m_path);
}

int CThreadPool::TaskDispatch()
{
	while (m_epoll != -1) {
		std::vector<epoll_event> events;
		int ret = 0;
		ssize_t esize = m_epoll.WaitEvents(events);
		if (esize > 0) {
			for (ssize_t i = 0; i < esize; i++) {
				if (events[i].events & EPOLLIN) {
						CSocketBase* pClient = nullptr;
					if (events[i].data.ptr == m_server) {
						ret = m_server->Link(&pClient);
						if (ret != 0)continue;
						ret = m_epoll.Add(*pClient, (EpollData)(void*)pClient);
						if (ret != 0) {
							delete pClient;
							continue;
						}
					}
					else {
						pClient = (CSocketBase*)events[i].data.ptr;
						if (pClient != nullptr) {
							CFunction* base = nullptr;
							Buffer data(sizeof(base));
							ret = pClient->Recv(data);
							if (ret <= 0) {
								m_epoll.Del(*pClient);
								delete pClient;
								continue;
							}
							memcpy(&base, (char*)data, sizeof(base));
							if (base != nullptr) {
								(*base)();
								delete base;
							}
						}
					}
				}
			}
		}
	}
	return 0;
}
