#pragma once
#include <cstdio>
#include <mutex>
#include <cstring>
#include <string>

#define BUF_SIZE 1024

#ifdef _DEBUG
extern FILE* pFile;
extern char szBufInfo[BUF_SIZE];
extern std::mutex debugMutex; // Add a mutex for synchronization
#endif // DEBUG

class Buffer :public std::string
{
public:
	Buffer() :std::string() {}
	Buffer(size_t size) :std::string() { resize(size); }
	Buffer(const std::string& str) :std::string(str) {}
	Buffer(const char* str) :std::string(str) {}
	Buffer(const char* str, size_t length);
	//不包含end
	Buffer(const char* begin, const char* end);
	operator void* () { return (char*)c_str(); }
	operator char* () { return (char*)c_str(); }
	operator char* () const { return (char*)c_str(); }
	operator unsigned char* () const { return (unsigned char*)c_str(); }
	operator const char* () const { return c_str(); }
	operator const void* () const { return c_str(); }
	Buffer& operator=(const char* str);
};

