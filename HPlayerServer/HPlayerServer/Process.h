#pragma once
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include "Function.h"

class CProcess {
public:
    CProcess() {
        m_func = nullptr;
        memset(pipes, 0, sizeof pipes);
    }

    ~CProcess() {
        if (m_func != nullptr)
            delete m_func;
    }

    template<typename Function, typename...Args>
    int SetEntryFunction(Function func, Args...args) {
        m_func = new CFunction(func, args...);
        return 0;
    }

    int CreateSubProcess() {
        if (m_func == nullptr)return -1;
        int ret = socketpair(AF_LOCAL, SOCK_STREAM, 0, pipes);
        if (ret == -1)return -2;
        pid_t pid = fork();
        if (pid == -1)return -3;
        else if (pid == 0) {//subprocess  read
            close(pipes[1]);//close write
            pipes[1] = 0;
            ret = (*m_func)();
            std::exit(0);
        }
        //mainprocess  write
        close(pipes[0]);//close read
        pipes[0] = 0;
        m_pid = pid;
        return 0;
    }

    int SendFD(int fd) {//主进程完成
        struct msghdr msg {};
        iovec iov[2];
        char buf[2][10] = { "edoyun","jueding" };
        iov[0].iov_base = buf[0];
        iov[0].iov_len = sizeof(buf[0]);
        iov[1].iov_base = buf[1];
        iov[1].iov_len = sizeof(buf[1]);
        msg.msg_iov = iov;
        msg.msg_iovlen = 2;

        // 下面的数据，才是我们需要传递的。
        cmsghdr* cmsg = (cmsghdr*)calloc(1, CMSG_LEN(sizeof(int)));
        if (cmsg == nullptr) return -1;
        cmsg->cmsg_len = CMSG_LEN(sizeof(int));
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;
        *(int*)CMSG_DATA(cmsg) = fd;
        msg.msg_control = cmsg;
        msg.msg_controllen = cmsg->cmsg_len;

        ssize_t ret = sendmsg(pipes[1], &msg, 0);
        free(cmsg);
        if (ret == -1) {
            return -2;
        }
        return 0;
    }

    int RecvFD(int& fd)
    {
        struct msghdr msg{};
        iovec iov[2];
        char buf[][10] = { "","" };
        iov[0].iov_base = buf[0];
        iov[0].iov_len = sizeof(buf[0]);
        iov[1].iov_base = buf[1];
        iov[1].iov_len = sizeof(buf[1]);
        msg.msg_iov = iov;
        msg.msg_iovlen = 2;

        cmsghdr* cmsg = (cmsghdr*)calloc(1, CMSG_LEN(sizeof(int)));
        if (cmsg == NULL)return -1;
        cmsg->cmsg_len = CMSG_LEN(sizeof(int));
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;
        msg.msg_control = cmsg;
        msg.msg_controllen = CMSG_LEN(sizeof(int));
        ssize_t ret = recvmsg(pipes[0], &msg, 0);
        if (ret == -1) {
            free(cmsg);
            return -2;
        }
        fd = *(int*)CMSG_DATA(cmsg);
        free(cmsg);
        return 0;
    }

    static int SwitchDeamon() {
        pid_t ret = fork();
        if (ret == -1) return -1;
        if (ret > 0) std::exit(0);  // 主进程到此为止

        // 子进程内容如下
        ret = setsid();
        if (ret == -1) return -2; // 失败，则返回

        ret = fork();
        if (ret == -1) return -3;
        if (ret > 0) std::exit(0);  // 子进程到此为止

        // 孙进程的内容如下，进入守护状态
        chdir("/");  // 修改工作目录到根目录
        // 关闭标准输入、标准输出和标准错误输出
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        umask(0);
        signal(SIGCHLD, SIG_IGN);
        return 0;
    }
private:
    CFunction* m_func;
    pid_t m_pid;
    int pipes[2];
};
