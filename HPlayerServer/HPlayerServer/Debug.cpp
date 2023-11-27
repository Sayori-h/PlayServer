#include "Debug.h"

FILE* pFile = fopen("./BugInfo.txt", "w+");
char szBufInfo[1024];
std::mutex debugMutex;

