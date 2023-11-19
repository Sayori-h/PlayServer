#include "CEpoll.h"

EpollData::EpollData()
{
	m_data.u64 = 0;
}

EpollData::EpollData(void* ptr)
{
	m_data.ptr = ptr;
}

EpollData::EpollData(int fd)
{
	m_data.fd = fd;
}

EpollData::EpollData(uint32_t u32)
{
	m_data.u32 = u32;
}

EpollData::EpollData(uint64_t u64)
{
	m_data.u64 = u64;
}

EpollData::EpollData(const EpollData& data)
{	
	m_data.u64 = data.m_data.u64;
}

EpollData& EpollData::operator=(const EpollData& data)
{
	if (this != &data) {
		m_data.u64 = data.m_data.u64;
	}
	return *this;
}

EpollData& EpollData::operator=(void* ptr)
{
	m_data.ptr = ptr;
	return *this;
}

EpollData& EpollData::operator=(int fd)
{
	m_data.fd = fd;
	return *this;
}

EpollData& EpollData::operator=(uint32_t u32)
{
	m_data.u32 = u32;
	return *this;
}

EpollData& EpollData::operator=(uint64_t u64)
{
	m_data.u64=u64;
	return *this;
}

EpollData::operator epoll_data_t()
{
	return m_data;
}

EpollData::operator epoll_data_t() const
{
	return m_data;
}

EpollData::operator epoll_data_t* ()
{
	return &m_data;
}

EpollData::operator const epoll_data_t* () const
{
	return &m_data;
}

CEpoll::CEpoll()
{
	m_epoll = -1;
}

CEpoll::~CEpoll()
{
	Close();
}

CEpoll::operator int() const
{
	return m_epoll;
}

int CEpoll::Create(unsigned count)
{
	if (m_epoll != -1)return -1;
	m_epoll = epoll_create(count);
	if (m_epoll == -1)return -2;
	return 0;
}

ssize_t CEpoll::WaitEvents(std::vector<epoll_event>& events, int timeout)
{
	if (m_epoll == -1)return -1;
	std::vector<epoll_event> evs(EVENT_SIZE);
	int ret = epoll_wait(m_epoll, reinterpret_cast<struct epoll_event*>(evs.data()),
		static_cast<int>(evs.size()), timeout);
	if (ret == -1) {
		if ((errno == EINTR) || (errno == EAGAIN))return 0;
		return -2;
	}
	if (ret > static_cast<int>(events.size()))
		events.resize(ret);
	memcpy(events.data(), evs.data(), sizeof(epoll_event) * ret);
	return ret;
}

int CEpoll::Add(int fd, const EpollData& data, uint32_t events)
{
	if (m_epoll == -1)return -1;
	epoll_event ev = { events,data };
	int ret = epoll_ctl(m_epoll, EPOLL_CTL_ADD, fd, &ev);
	if (ret == -1)return -2;
	return 0;
}

int CEpoll::Modify(int fd, uint32_t events, const EpollData& data)
{
	if (m_epoll == -1)return -1;
	epoll_event ev = { events,data };
	int ret = epoll_ctl(m_epoll, EPOLL_CTL_MOD, fd, &ev);
	if (ret == -1)return -2;
	return 0;
}

int CEpoll::Del(int fd)
{
	if (m_epoll == -1)return -1;
	int ret = epoll_ctl(m_epoll, EPOLL_CTL_DEL, fd, nullptr);
	if (ret == -1)return -2;
	return 0;
}

void CEpoll::Close()
{
	if (m_epoll != -1) {
		int fd = m_epoll;
		m_epoll = -1;
		close(fd);
	}
}
