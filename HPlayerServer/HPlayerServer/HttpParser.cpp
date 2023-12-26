#include "HttpParser.h"

CHttpParser::CHttpParser()
{
	m_complete = false;
	memset(&m_parser, 0, sizeof(m_parser));
	m_parser.data = this;
	http_parser_init(&m_parser, HTTP_REQUEST);
	memset(&m_settings, 0, sizeof(m_settings));
	m_settings.on_message_begin = &CHttpParser::OnMessageBegin;
	m_settings.on_url = &CHttpParser::OnUrl;
	m_settings.on_status = &CHttpParser::OnStatus;
	m_settings.on_header_field = &CHttpParser::OnHeaderField;
	m_settings.on_header_value = &CHttpParser::OnHeaderValue;
	m_settings.on_headers_complete = &CHttpParser::OnHeadersComplete;
	m_settings.on_body = &CHttpParser::OnBody;
	m_settings.on_message_complete = &CHttpParser::OnMessageComplete;
}

CHttpParser::~CHttpParser()
{
}

CHttpParser::CHttpParser(const CHttpParser& http)
{
	memcpy(&m_parser, &http.m_parser, sizeof(m_parser));
	m_parser.data = this;
	memcpy(&m_settings, &http.m_settings, sizeof(m_settings));
	m_status = http.m_status;
	m_url = http.m_url;
	m_body = http.m_body;
	m_complete = http.m_complete;
	m_lastField = http.m_lastField;
}

CHttpParser& CHttpParser::operator=(const CHttpParser& http)
{
	if (this != &http) {
		memcpy(&m_parser, &http.m_parser, sizeof(m_parser));
		m_parser.data = this;
		memcpy(&m_settings, &http.m_settings, sizeof(m_settings));
		m_status = http.m_status;
		m_url = http.m_url;
		m_body = http.m_body;
		m_complete = http.m_complete;
		m_lastField = http.m_lastField;
	}
	return *this;
}

size_t CHttpParser::ExecParser(const Buffer& data)
{
	m_complete = false;
	size_t ret = http_parser_execute(&m_parser, &m_settings, data, data.size());
	if (m_complete == false) {
		m_parser.http_errno = 0x7F;
		return 0;
	}
	return ret;
}

unsigned CHttpParser::Method() const
{
	return m_parser.method;
}

const std::map<Buffer, Buffer>& CHttpParser::Header()
{
	return m_headerValues;
}

const Buffer& CHttpParser::Status() const
{
	return m_status;
}

const Buffer& CHttpParser::Url() const
{
	return m_url;
}

const Buffer& CHttpParser::Body() const
{
	return m_body;
}

unsigned CHttpParser::Errno() const
{
	return m_parser.http_errno;
}

int CHttpParser::OnMessageBegin(http_parser* parser)
{
	return ((CHttpParser*)parser->data)->OnMessageBegin();
}

int CHttpParser::OnUrl(http_parser* parser, const char* at, size_t length)
{
	return ((CHttpParser*)parser->data)->OnUrl(at,length);
}

int CHttpParser::OnStatus(http_parser* parser, const char* at, size_t length)
{
	return ((CHttpParser*)parser->data)->OnStatus(at, length);
}

int CHttpParser::OnHeaderField(http_parser* parser, const char* at, size_t length)
{
	return ((CHttpParser*)parser->data)->OnHeaderField(at, length);
}

int CHttpParser::OnHeaderValue(http_parser* parser, const char* at, size_t length)
{
	return ((CHttpParser*)parser->data)->OnHeaderValue(at, length);
}

int CHttpParser::OnHeadersComplete(http_parser* parser)
{
	return ((CHttpParser*)parser->data)->OnHeadersComplete();
}

int CHttpParser::OnBody(http_parser* parser, const char* at, size_t length)
{
	return ((CHttpParser*)parser->data)->OnBody(at, length);
}

int CHttpParser::OnMessageComplete(http_parser* parser)
{
	return ((CHttpParser*)parser->data)->OnMessageComplete();
}

int CHttpParser::OnMessageBegin()
{
	return 0;
}

int CHttpParser::OnUrl(const char* at, size_t length)
{
	m_url = Buffer(at, length);
	return 0;
}

int CHttpParser::OnStatus(const char* at, size_t length)
{
	m_status = Buffer(at, length);
	return 0;
}

int CHttpParser::OnHeaderField(const char* at, size_t length)
{
	m_lastField = Buffer(at, length);
	return 0;
}

int CHttpParser::OnHeaderValue(const char* at, size_t length)
{
	m_headerValues[m_lastField] = Buffer(at, length);
	return 0;
}

int CHttpParser::OnHeadersComplete()
{
	return 0;
}

int CHttpParser::OnBody(const char* at, size_t length)
{
	m_body = Buffer(at, length);
	return 0;
}

int CHttpParser::OnMessageComplete()
{
	m_complete = true;
	return 0;
}

UrlParser::UrlParser(const Buffer& url)
{
	m_url = url;
}

UrlParser::~UrlParser()
{
}

int UrlParser::ExecParser()
{
	//http ://www.luffycity.com:80 /news/index.html?id=250&page=1
	const char* pos = m_url;
	//解析协议
	const char* target = strstr(pos, "://");
	if (target == nullptr)return -1;
	m_protocol = Buffer(pos, target);//http
	//解析域名和端口
	pos = target + 3;
	target = strchr(pos, '/');
	if (target == nullptr) {//http: //www.luffycity.com
		if (m_protocol.size() + 3 >= m_url.size())return -2;
		m_host = pos;
		return 0;
	}
	Buffer value(pos, target);//www.luffycity.com:80
	if (value.empty())return -3;
	target = strchr(value, ':');
	if (target != nullptr) {
		m_host = Buffer(value, target);//www.luffycity.com
		m_port = atoi(Buffer(target + 1, (char*)value + value.size()));
	}
	else m_host = value;
	//解析URI   /news/index.html?id=250&page=1
	pos = strchr(pos, '/');
	target = strchr(pos, '?');
	if (target == nullptr) {
		m_uri = pos;
	}
	else {//     ?id=250&page=1
		m_uri = Buffer(pos, target);
		//解析key和value
		pos = target + 1;
		const char* temp = nullptr;
		do
		{
			target = strchr(pos, '&');
			if (target == nullptr) {//page = 1
				temp = strchr(pos, '=');
				if (temp == nullptr)return -4;
				m_values[Buffer(pos, temp)] = Buffer(temp + 1);
			}
			else {//id=250
				Buffer kv(pos, target);
				temp = strchr(kv, '=');
				if (temp == nullptr)return -5;
				m_values[Buffer(kv, temp)] = Buffer(temp+1,(char*)kv+kv.size());
				pos = target + 1; 
			}
		} while (target!=nullptr);
	}
	return 0;
}

Buffer UrlParser::operator[](const Buffer& name) const
{
	auto it = m_values.find(name);
	if (it == m_values.end())return Buffer();
	return it->second;
}

Buffer UrlParser::Protocol() const
{
	return m_protocol;
}

Buffer UrlParser::Host() const
{
	return m_host;
}

const Buffer UrlParser::Uri() const
{
	return m_uri;
}

int UrlParser::Port() const
{
	return m_port;
}

void UrlParser::SetUrl(const Buffer& url)
{
	m_url = url;
	m_protocol = "";
	m_host = "";
	m_port = 80;
	m_values.clear();
}
