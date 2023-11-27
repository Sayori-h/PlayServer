#pragma once
#include <unistd.h>
#include <sys/epoll.h>
#include <vector>
#include <cstring>
#include <cerrno>

#define EVENT_SIZE 256

class EpollData {
public:
    EpollData();
    EpollData(void* ptr);
    explicit EpollData(int fd);
    explicit EpollData(uint32_t u32);
    explicit EpollData(uint64_t u64);
    EpollData(const EpollData& data);

    EpollData& operator=(const EpollData& data);
    EpollData& operator=(void* ptr);
    EpollData& operator=(int fd);
    EpollData& operator=(uint32_t u32);
    EpollData& operator=(uint64_t u64);

    operator epoll_data_t();
    operator epoll_data_t() const;
    operator epoll_data_t*();
    operator const epoll_data_t*() const;
private:
    epoll_data_t m_data;
};

class CEpoll
{
public:
    CEpoll();
    ~CEpoll();
    operator int()const;
public:
    CEpoll(const CEpoll&) = delete;
    CEpoll& operator=(const CEpoll&) = delete;
public:
    int Create(unsigned count);
    //小于0表示错误等于0表示没有事情发生大于0表示成功拿到事件
    ssize_t WaitEvents(std::vector<epoll_event>& events, int timeout = 10);
    int Add(int fd, const EpollData& data = EpollData((void*)0), uint32_t events = EPOLLIN);
    int Modify(int fd, uint32_t events, const EpollData& data = EpollData((void*)0));
    int Del(int fd);
    void Close();
private:
    int m_epoll;
};

