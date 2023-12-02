#include "HPlayServer.h"

int CreateLogServer(CProcess* proc) {
    CLoggerServer server;
    int ret=server.Start();//开启日志服务器
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
#endif // DEBUG 
    }
    int fd = 0;
    while (true) {
        ret=proc->RecvFD(fd);//等待主进程发送终止信号
#ifdef _DEBUG
        {
            std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
            memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
            snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> ret=%d fd=%d\n",
                __FILE__, __LINE__, __FUNCTION__, ret, fd);
            fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
            fflush(pFile);
        }
#endif // DEBUG 
        if (fd <=0)break;
    }
    ret=server.Close();
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
    return 0;
}

int CreateClientServer(CProcess* proc) {
#ifdef DEBUG
    {
        std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
        memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
        snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> pid=%d\n", __FILE__, __LINE__, __FUNCTION__, getpid());
        fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
        fflush(pFile);
    }
#endif // DEBUG  
    int fd = -1;
    /*int ret = */proc->RecvFD(fd);
#ifdef DEBUG
    {
        std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
        memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
        snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);
        snprintf(szBufInfo + strlen(szBufInfo), BUF_SIZE - strlen(szBufInfo),
            "%s(%d):<%s> fd=%d\n", __FILE__, __LINE__, __FUNCTION__, fd);
        fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
        fflush(pFile);
    }
#endif // DEBUG  
    sleep(1);
    char buf[20] = "";
    lseek(fd, 0, SEEK_SET);
    read(fd, buf, sizeof(buf));
#ifdef DEBUG
    {
        std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
        memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
        snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> buf=%s\n", __FILE__, __LINE__, __FUNCTION__, buf);
        fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
        fflush(pFile);
    }
#endif // DEBUG 
    close(fd);
    return 0;
}

int LogTest()
{
    char buffer[] = "hello world! 恕瑞玛";
    usleep(1000 * 100);
    TRACE_INFO("here is log %d %c %f %g %s 哈哈 日志第一条", 10, 'A', 1.0f, 2.0, buffer);
    DUMP_DEBUG((void*)buffer, (size_t)sizeof(buffer));
    LOG_ERROR << 100 << " " << 'S' << " " << 0.12345f
        << " " << 1.23456789 << " " << buffer << " 日志最后一条";
    return 0;
}

int OldTest() {
    //CProcess::SwitchDeamon();
    CProcess proclog, proclients;
#ifdef _DEBUG
    {
        std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
        memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
        snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> pid=%d\n",
            __FILE__, __LINE__, __FUNCTION__, getpid());
        fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
        fflush(pFile);
    }
#endif // DEBUG  
    proclog.SetEntryFunction(CreateLogServer, &proclog);
    int ret = proclog.CreateSubProcess();
    if (ret) {
#ifdef _DEBUG
        {
            std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
            memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
            snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> pid=%d\n",
                __FILE__, __LINE__, __FUNCTION__, getpid());
            fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
            fflush(pFile);
        }
#endif // DEBUG  
        return -1;
    }

    LogTest();

#ifdef _DEBUG
    {
        std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
        memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
        snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> pid=%d\n",
            __FILE__, __LINE__, __FUNCTION__, getpid());
        fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
        fflush(pFile);
    }
#endif // DEBUG  

    CThread thread(LogTest);
    thread.Start();

    proclients.SetEntryFunction(CreateClientServer, &proclients);
    ret = proclients.CreateSubProcess();
    if (ret) {
#ifdef _DEBUG
        {
            std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
            memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
            snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> pid=%d\n",
                __FILE__, __LINE__, __FUNCTION__, getpid());
            fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
            fflush(pFile);
        }
#endif // DEBUG  
        return -2;
    }
#ifdef _DEBUG
    {
        std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
        memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
        snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> pid=%d\n",
            __FILE__, __LINE__, __FUNCTION__, getpid());
        fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
        fflush(pFile);
    }
#endif // DEBUG  
    sleep(1);
    int fd = open("./testFD.txt", O_RDWR | O_CREAT | O_APPEND);
#ifdef _DEBUG
    {
        std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
        memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
        snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> fd=%d\n",
            __FILE__, __LINE__, __FUNCTION__, fd);
        fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
        fflush(pFile);
    }
#endif // DEBUG  
    if (fd == -1)return -3;
    ret = proclients.SendFD(fd);
    if (ret != 0) {
#ifdef _DEBUG
        {
            std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
            memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
            snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> ret=%d\n",
                __FILE__, __LINE__, __FUNCTION__, ret);
            snprintf(szBufInfo + strlen(szBufInfo), BUF_SIZE - strlen(szBufInfo),
                "errno:%d msg:%s\n", errno, strerror(errno));
            fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
            fflush(pFile);
        }
#endif // DEBUG  
        return -4;
    }
    write(fd, "Fd Send Test...\n", 16);
    close(fd);

    CThreadPool pool;
    ret = pool.Start(4);
    if (ret != 0) {
#ifdef _DEBUG
        {
            std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
            memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
            snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> ret=%d\n",
                __FILE__, __LINE__, __FUNCTION__, ret);
            snprintf(szBufInfo + strlen(szBufInfo), BUF_SIZE - strlen(szBufInfo),
                "errno:%d msg:%s\n", errno, strerror(errno));
            fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
            fflush(pFile);
        }
#endif // DEBUG  
    }
    ret = pool.AddTask(LogTest);
    if (ret != 0) {
#ifdef _DEBUG
        {
            std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
            memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
            snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> ret=%d\n",
                __FILE__, __LINE__, __FUNCTION__, ret);
            snprintf(szBufInfo + strlen(szBufInfo), BUF_SIZE - strlen(szBufInfo),
                "errno:%d msg:%s\n", errno, strerror(errno));
            fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
            fflush(pFile);
        }
#endif // DEBUG  
    }
    ret = pool.AddTask(LogTest);
    if (ret != 0) {
#ifdef _DEBUG
        {
            std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
            memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
            snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> ret=%d\n",
                __FILE__, __LINE__, __FUNCTION__, ret);
            snprintf(szBufInfo + strlen(szBufInfo), BUF_SIZE - strlen(szBufInfo),
                "errno:%d msg:%s\n", errno, strerror(errno));
            fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
            fflush(pFile);
        }
#endif // DEBUG  
    }
    ret = pool.AddTask(LogTest);
    if (ret != 0) {
#ifdef _DEBUG
        {
            std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
            memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
            snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> ret=%d\n",
                __FILE__, __LINE__, __FUNCTION__, ret);
            snprintf(szBufInfo + strlen(szBufInfo), BUF_SIZE - strlen(szBufInfo),
                "errno:%d msg:%s\n", errno, strerror(errno));
            fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
            fflush(pFile);
        }
#endif // DEBUG  
    }
    ret = pool.AddTask(LogTest);
    if (ret != 0) {
#ifdef _DEBUG
        {
            std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
            memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
            snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> ret=%d\n",
                __FILE__, __LINE__, __FUNCTION__, ret);
            snprintf(szBufInfo + strlen(szBufInfo), BUF_SIZE - strlen(szBufInfo),
                "errno:%d msg:%s\n", errno, strerror(errno));
            fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
            fflush(pFile);
        }
#endif // DEBUG  
    }

    fclose(pFile);
    (void)getchar();
    pool.Close();
    proclog.SendFD(-1);
    (void)getchar();
    return 0;
}

int main()
{
    int ret = 0;
    CProcess proclog;
    ret = proclog.SetEntryFunction(CreateLogServer, &proclog);
    ERR_RETURN(ret, -1);
    ret = proclog.CreateSubProcess();
    ERR_RETURN(ret, -2);
    CHPlayServer business(4);
    CServer server;
    ret = server.Init(&business);
    ERR_RETURN(ret, -3);
    ret = server.Run();
    ERR_RETURN(ret, -4);
    return 0;
}