#pragma once
#include <cstdio>

#define DEBUG
#define BUF_SIZE 1024

#ifdef DEBUG
extern FILE* pFile;
extern char szBufInfo[BUF_SIZE];

#endif // DEBUG

