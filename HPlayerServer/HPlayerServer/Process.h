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

    int SendFD(int fd) {//mainprocess
        struct msghdr msg = {};
        struct iovec iov[2];
        iov[0].iov_base = (char*)"edoyun";
        iov[0].iov_len = 7;
        iov[1].iov_base = (char*)"jueding";
        iov[1].iov_len = 8;

        cmsghdr* cmsg = new cmsghdr();
        memset(cmsg, 0, sizeof(*cmsg));
        if (cmsg == nullptr) return -1;

        cmsg->cmsg_len = CMSG_LEN(sizeof(int));
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;
        *(int*)CMSG_DATA(cmsg) = fd;

        msg.msg_control = cmsg;
        msg.msg_controllen = cmsg->cmsg_len;
        msg.msg_iov = iov;
        msg.msg_iovlen = 2;

        ssize_t ret = sendmsg(pipes[1], &msg, 0);
        delete cmsg;

        if (ret == -1) return -2;
        return 0;
    }

    int RecvFD(int& fd) {//subprocess
        struct msghdr msg = {};
        struct iovec iov[2];
        char buf[][10] = { "","" };
        iov[0].iov_base = buf[0];
        iov[0].iov_len = sizeof(buf[0]);
        iov[1].iov_base = buf[1];
        iov[1].iov_len = sizeof(buf[1]);

        cmsghdr* cmsg = new cmsghdr();
        memset(cmsg, 0, sizeof(*cmsg));
        if (cmsg == nullptr) return -1;

        cmsg->cmsg_len = CMSG_LEN(sizeof(int));
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;

        msg.msg_control = cmsg;
        msg.msg_controllen = cmsg->cmsg_len;
        msg.msg_iov = iov;
        msg.msg_iovlen = 2;

        ssize_t ret = recvmsg(pipes[0], &msg, 0);

        if (ret == -1) return -2;
        fd = *(int*)CMSG_DATA(cmsg);

        delete cmsg;
        return 0;
    }

    static int SwitchDeamon() {
        pid_t ret = fork();
        if (ret == -1) return -1;
        if (ret > 0) std::exit(0);  // �����̵���Ϊֹ

        // �ӽ�����������
        ret = setsid();
        if (ret == -1) return -2; // ʧ�ܣ��򷵻�

        ret = fork();
        if (ret == -1) return -3;
        if (ret > 0) std::exit(0);  // �ӽ��̵���Ϊֹ

        // ����̵��������£������ػ�״̬
        chdir("/");  // �޸Ĺ���Ŀ¼����Ŀ¼
        // �رձ�׼���롢��׼����ͱ�׼�������
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
