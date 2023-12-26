#pragma once
#include "http_parser.h"
#include "Socket.h"
#include <vector>
#include <map>

class CHttpParser
{
public:
	CHttpParser();
	~CHttpParser();
	CHttpParser(const CHttpParser& http);
	CHttpParser& operator=(const CHttpParser& http);
public:
	size_t ExecParser(const Buffer& data);
	unsigned Method()const;
	const std::map<Buffer, Buffer>&Header();
	const Buffer& Status()const;
	const Buffer& Url()const;
	const Buffer& Body()const;
	unsigned Errno()const;
protected:	
	static int OnMessageBegin(http_parser* parser);//消息开始
	static int OnUrl(http_parser* parser, const char* at, size_t length);//URL解析
	static int OnStatus(http_parser* parser, const char* at, size_t length);//状态解析
	static int OnHeaderField(http_parser* parser, const char* at, size_t length);//头部字段解析
	static int OnHeaderValue(http_parser* parser, const char* at, size_t length);//头部值解析
	static int OnHeadersComplete(http_parser* parser);//头部解析完成
	static int OnBody(http_parser* parser, const char* at, size_t length);//消息体解析
	static int OnMessageComplete(http_parser* parser);//消息完成

	int OnMessageBegin();
	int OnUrl(const char* at, size_t length);
	int OnStatus(const char* at, size_t length);
	int OnHeaderField(const char* at, size_t length);
	int OnHeaderValue(const char* at, size_t length);
	int OnHeadersComplete();
	int OnBody(const char* at, size_t length);
	int OnMessageComplete();
private:
	http_parser m_parser;
	http_parser_settings m_settings;
	std::map<Buffer, Buffer> m_headerValues;
	Buffer m_status;
	Buffer m_url;
	Buffer m_body;
	bool m_complete;
	Buffer m_lastField;
};

class UrlParser
{
public:
	UrlParser(const Buffer& url);
	~UrlParser();
	int ExecParser();
	Buffer operator[](const Buffer& name)const;
	Buffer Protocol()const;
	Buffer Host()const;
	const Buffer Uri()const;
	int Port()const;
	void SetUrl(const Buffer& url);
private:
	Buffer m_url;
	Buffer m_protocol;
	Buffer m_host;
	Buffer m_uri;
	int m_port;
	std::map<Buffer, Buffer>m_values;
};
