#pragma once
#include <cstdio>
#include <mutex>
#include <cstring>
#define BUF_SIZE 1024

#ifdef _DEBUG
extern FILE* pFile;
extern char szBufInfo[BUF_SIZE];
extern std::mutex debugMutex; // Add a mutex for synchronization
#endif // DEBUG

