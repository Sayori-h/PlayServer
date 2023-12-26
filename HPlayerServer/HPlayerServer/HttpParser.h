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
	static int OnMessageBegin(http_parser* parser);//��Ϣ��ʼ
	static int OnUrl(http_parser* parser, const char* at, size_t length);//URL����
	static int OnStatus(http_parser* parser, const char* at, size_t length);//״̬����
	static int OnHeaderField(http_parser* parser, const char* at, size_t length);//ͷ���ֶν���
	static int OnHeaderValue(http_parser* parser, const char* at, size_t length);//ͷ��ֵ����
	static int OnHeadersComplete(http_parser* parser);//ͷ���������
	static int OnBody(http_parser* parser, const char* at, size_t length);//��Ϣ�����
	static int OnMessageComplete(http_parser* parser);//��Ϣ���

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
