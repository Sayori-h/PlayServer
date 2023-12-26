#include "HPlayServer.h"

CHPlayServer::CHPlayServer(unsigned count)
	:CBusiness()
{
	m_count = count;
}

CHPlayServer::~CHPlayServer()
{
	m_epoll.Close();
	m_pool.Close();
	for (auto it : m_mapClients)
		if (it.second != nullptr) {
			delete it.second;
		}
	m_mapClients.clear();
}

int CHPlayServer::BusinessProcess(CProcess* proc)
{
	using namespace std::placeholders;
	int ret = 0;
	m_db = new CMysqlClient();
	if (m_db == nullptr) {
		TRACE_ERROR("no more memory!\n");
		return -1;
	}
	std::map<Buffer, Buffer> args;
	args["host"] = "192.168.6.128";
	args["user"] = "root";
	args["password"] = "hu20010618";
	args["port"] = 3306;
	args["db"] = "HPlayer";
	ret = m_db->Connect(args);
	ERR_RETURN(ret, -2);
	huxlLogin_user_mysql user;
	ret = m_db->Exec(user.TCreate());
	ERR_RETURN(ret, -3);
	ret = setConnected(&CHPlayServer::Connected, this, _1);
	ERR_RETURN(ret, -4);
	ret = setRecvDone(&CHPlayServer::Received, this, _1, _2);
	ERR_RETURN(ret, -5);
	ret = m_epoll.Create(m_count);
	ERR_RETURN(ret, -6);
	ret = m_pool.Start(m_count);
	ERR_RETURN(ret, -7);
	for (unsigned i = 0; i < m_count; i++) {
		ret = m_pool.AddTask(&CHPlayServer::ThreadFunc, this);
		ERR_RETURN(ret, -8);
	}
	int sock = 0;
	sockaddr_in addrin;
	while (m_epoll != -1) {//检测状态
		ret = proc->RecvSocket(sock,&addrin);
		if (ret < 0 || (sock == 0))break;
		CSocketBase* pClient = new CSocket(sock);
		if (pClient == nullptr)continue;
		ret = pClient->Init(CSockParam(&addrin,SOCK_ISIP));
		WARN_CONTINUE(ret);
		ret = m_epoll.Add(sock, (EpollData)(void*)pClient);
		if (m_connected) {
			(*m_connected)(pClient);
		}
		WARN_CONTINUE(ret);
	}
	return 0;
}

int CHPlayServer::ThreadFunc()
{
	int ret = 0;
	std::vector<epoll_event> events;
	while ((m_epoll != -1)) {
		ssize_t size = m_epoll.WaitEvents(events);
		if (size < 0)break;
		if (size > 0) {
			for (ssize_t i = 0; i < size; i++) {
				if (events[i].events & EPOLLERR)break;
				else if (events[i].events & EPOLLIN) {//处理读取事件
					CSocketBase* pClient = (CSocketBase*)events[i].data.ptr;
					if (pClient != nullptr) {
						Buffer data;
						ret = pClient->Recv(data);
						WARN_CONTINUE(ret);
						if (m_recvdone != nullptr)
							(*m_recvdone)(pClient,data);
					}					
				}
			}
		}
	}
	return 0;
}

int CHPlayServer::Connected(CSocketBase* pClient)
{
	//TODO：客户端连接处理：简单打印一下客户端信息
	sockaddr_in* paddr = *pClient;
	TRACE_INFO("client connected addr %s port:%d",
		inet_ntoa(paddr->sin_addr), paddr->sin_port);
	return 0;
}

int CHPlayServer::Received(CSocketBase* pClient, const Buffer& data)
{
	//TODO:主要业务，在此处理
	//HTTP 解析
	int ret = 0;
	Buffer response = "";
	ret = HttpParser(data);
	//验证结果的反馈
	if (ret != 0) {//验证失败
		TRACE_ERROR("http parser failed!%d", ret);
		return -1;
	}
	//验证成功
	response = MakeResponse(ret);
	ret = pClient->Send(response);
	if (ret != 0) 
		TRACE_ERROR("http response failed!%d", ret);
	else 
		TRACE_INFO("http response success!%d", ret);
	return 0;
}

int CHPlayServer::HttpParser(const Buffer& data)
{
	CHttpParser parser;
	size_t size = parser.ExecParser(data);
	if (!size || parser.Errno()) {
		TRACE_ERROR("size %llu errno:%u", size, parser.Errno());
		return -1;
	}
	if (parser.Method() == HTTP_GET) {
		//get处理
		Buffer burl = "https://192.168.6.128" + parser.Url();
		UrlParser url(burl);
		int ret = url.ExecParser();
		if (ret != 0) {
			TRACE_ERROR("ret=%d url [%s]",ret,burl);
			return -2;
		}
		Buffer uri = url.Uri();
		if (uri == "login") {
			//处理登录
			Buffer time = url["time"];
			Buffer salt = url["salt"];
			Buffer user = url["user"];
			Buffer sign = url["sign"];
			TRACE_INFO("time %s salt %s user %s sign %s", 
				(char*)time, (char*)salt, (char*)user, (char*)sign);
			//数据库的查询
			huxlLogin_user_mysql dbuser;
			std::list<PTable> result;
			Buffer sql = dbuser.Query("user_name=\"" + user + "\"");
			ret = m_db->Exec(sql, result, dbuser);
			if (ret != 0) {
				TRACE_ERROR("sql=%s ret=%d", (char*)sql, ret);
				return -3;
			}
			if (result.size() == 0) {
				TRACE_ERROR("no result sql=%s ret=%d", (char*)sql, ret);
				return -4;
			}
			else if (result.size() != 1) {
				TRACE_ERROR("more than one sql=%s ret=%d", (char*)sql, ret);
				return -5;
			}
			auto user1 = result.front();
			Buffer pwd = user1->MapFields.at((Buffer)"user_password")->Value.String;
			TRACE_INFO("password=%s", (char*)pwd);
			//登录请求的验证
			const char* MD5_KEY= "*&^%$#@b.v+h-b*g/h@n!h#n$d^ssx,.kl<kl";
			Buffer md5str = time + MD5_KEY + pwd + salt;
			Buffer md5 = Crypto::MD5(md5str);
			TRACE_INFO("md5 = %s", (char*)md5);
			if (md5 == sign)return 0;
			return -6;
		}
	}
	else if (parser.Method() == HTTP_POST) {
		//post 处理
	}
	return -7;
}

Buffer CHPlayServer::MakeResponse(int ret)
{
	Json::Value root;
	root["status"] = ret;
	if (ret != 0)
		root["message"] = "登录失败，用户名或密码错误！";
	else
		root["message"] = "success";
	Buffer json = root.toStyledString();
	Buffer result = "HTTP/1.1 200 OK";
	time_t t;
	time(&t);
	tm* ptm = localtime(&t);
	char temp[64] = "";
	strftime(temp, sizeof(temp), "%a, %d %b %G %T GMT\r\n",ptm);
	Buffer Date = (Buffer)"Date: " + temp;
	Buffer Server = "Server: Edoyun/1.0\r\nContent-Type: text/html; charset=utf-8\r\nX-Frame-Options: DENY\r\n";
	snprintf(temp, sizeof(temp), "%d", json.size());
	Buffer Length = Buffer("Content-Length: ") + temp + "\r\n";
	Buffer Stub = "X-Content-Type-Options: nosniff\r\nReferrer-Policy: same-origin\r\n\r\n";
	result += Date + Server + Length + Stub + json;
	TRACE_INFO("response:%s", (char*)result);
	return result;
}
