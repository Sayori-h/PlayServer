#include <cerrno>
#include "Function.h"
#include "Process.h"
#include "Debug.h"

int CreateLogServer(CProcess* proc) {
#ifdef DEBUG
    snprintf(szBufInfo,BUF_SIZE,"%s(%d):<%s> pid=%d\n", __FILE__, __LINE__, __FUNCTION__, getpid());
    fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
    fflush(pFile);
#endif // DEBUG  
    return 0;
}

int CreateClientServer(CProcess* proc) {
#ifdef DEBUG
    snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> pid=%d\n", __FILE__, __LINE__, __FUNCTION__, getpid());
    fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
    fflush(pFile);
#endif // DEBUG  
    int fd = -1;
    int ret = proc->RecvFD(fd);
#ifdef DEBUG
    snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);
    snprintf(szBufInfo + strlen(szBufInfo), BUF_SIZE - strlen(szBufInfo), 
        "%s(%d):<%s> fd=%d\n", __FILE__, __LINE__, __FUNCTION__, fd);
    fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
    fflush(pFile);
#endif // DEBUG  
    sleep(1);
    char buf[20] = "";
    lseek(fd, 0, SEEK_SET);
    read(fd, buf, sizeof(buf));
#ifdef DEBUG
    snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> buf=%s\n", __FILE__, __LINE__, __FUNCTION__, buf);
    fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
    fflush(pFile);
#endif // DEBUG 
    close(fd);
    return 0;
}


int main()
{
    //CProcess::SwitchDeamon();
    CProcess proclog, proclients;
#ifdef DEBUG
    snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> pid=%d\n", __FILE__, __LINE__, __FUNCTION__, getpid());
    fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
    fflush(pFile);
#endif // DEBUG  
    proclog.SetEntryFunction(CreateLogServer, &proclog);
    int ret = proclog.CreateSubProcess();
    if (ret) {
#ifdef DEBUG
        snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> pid=%d\n", __FILE__, __LINE__, __FUNCTION__, getpid());
        fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
        fflush(pFile);
#endif // DEBUG  
        return -1;
    }
#ifdef DEBUG
    snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> pid=%d\n", __FILE__, __LINE__, __FUNCTION__, getpid());
    fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
    fflush(pFile);
#endif // DEBUG  
    proclients.SetEntryFunction(CreateClientServer, &proclients);
    ret = proclients.CreateSubProcess();
    if (ret) {
#ifdef DEBUG
        snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> pid=%d\n", __FILE__, __LINE__, __FUNCTION__, getpid());
        fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
        fflush(pFile);
#endif // DEBUG  
        return -2;
    }
#ifdef DEBUG
    snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> pid=%d\n", __FILE__, __LINE__, __FUNCTION__, getpid());
    fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
    fflush(pFile);
#endif // DEBUG  
    sleep(1);
    int fd = open("./testFD.txt", O_RDWR | O_CREAT|O_APPEND);
#ifdef DEBUG
    snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> fd=%d\n", __FILE__, __LINE__, __FUNCTION__, fd);
    fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
    fflush(pFile);
#endif // DEBUG  
    if (fd == -1)return -3;
    ret = proclients.SendFD(fd);
    if (ret != 0) {
#ifdef DEBUG
    snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);
    snprintf(szBufInfo+strlen(szBufInfo), BUF_SIZE-strlen(szBufInfo), "errno:%d msg:%s\n",errno, strerror(errno));
    fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
    fflush(pFile);
#endif // DEBUG  
        return -4;
    }
    write(fd, "Fd Send Test...\n", 16);
    close(fd);
    fclose(pFile);
    return 0;
}