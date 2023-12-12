#include "Public.h"

FILE* pFile = fopen("./BugInfo.txt", "w+");
char szBufInfo[1024];
std::mutex debugMutex;

Buffer::Buffer(const char* str, size_t length) 
	:std::string() 
{
	resize(length);
	memcpy((char*)c_str(), str, length);
}

Buffer::Buffer(const char* begin, const char* end) 
	:std::string() 
{
	long int len = end - begin;
	if (len > 0) {
		resize(len);
		memcpy((char*)c_str(), begin, len);
	}
}

Buffer& Buffer::operator=(const char* str)
{
	resize(strlen(str));
	strcpy((char*)data(), str);
	return *this;
}
