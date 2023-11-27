#include "Socket.h"

CSockParam::CSockParam()
{
	ip = "";
	port = -1;
	attr = 0;
	memset(addrin(), 0, sizeof(addr_in));
	memset(addrun(), 0, sizeof(addr_un));
}

CSockParam::CSockParam(const CSockParam& param)
{
	this->ip = param.ip;
	this->port = param.port;
	this->attr = param.attr;
	memcpy(addrin(), &param.addr_in, sizeof(addr_in));
	memcpy(addrun(), &param.addr_un, sizeof(addr_un));
}

CSockParam::CSockParam(const Buffer& ip, short port, int attr)
{
	this->ip = ip;
	this->port = port;
	this->attr = attr;
	addr_in.sin_family = AF_INET;
	addr_in.sin_port = port;
	addr_in.sin_addr.s_addr = inet_addr(ip);
}

CSockParam::CSockParam(const Buffer& path, int attr)
{
	this->ip = path;
	this->attr = attr;
	addr_un.sun_family = AF_UNIX;
	strcpy(addr_un.sun_path, path);
}

CSockParam& CSockParam::operator=(const CSockParam& param)
{
	if (this != &param) {
		this->ip = param.ip;
		this->port = param.port;
		this->attr = param.attr;
		memcpy(addrin(), &param.addr_in, sizeof(addr_in));
		memcpy(addrun(), &param.addr_un, sizeof(addr_un));
	}
	return *this;
}

CSocketBase::CSocketBase()
{
	m_socket = -1;
	m_status = 0;
}

CSocketBase::~CSocketBase()
{
	Close();
}

void CSocketBase::Close()
{
	m_status = 3;
	if (m_socket != -1) {
		unlink(m_param.ip);
		int fd = m_socket;
		m_socket = -1;
		close(fd);
	}
}

int CLocalSocket::Init(const CSockParam& param)
{
	if (m_status != 0)return -1;
	m_param = param;
	int type = m_param.attr & SOCK_ISUDP ? SOCK_DGRAM : SOCK_STREAM;
	if (m_socket == -1)
		m_socket = socket(PF_LOCAL, type, 0);
	else m_status = 2;//accept来的套接字，已经处于连接状态
	if (m_socket == -1)return -2;
	int ret = 0;
	if (m_param.attr & SOCK_ISSERVER) {
		ret = bind(m_socket, m_param.addrun(), sizeof(sockaddr_un));
		if (ret == -1)return -3;
		ret = listen(m_socket, 32);
		if (ret == -1)return -4;
	}
	if (m_param.attr & SOCK_ISNONBLOCK) {
		int option = fcntl(m_socket, F_GETFL);
		if (option == -1)return -5;
		option |= O_NONBLOCK;
		ret = fcntl(m_socket, F_SETFL, option);
		if (ret == -1)return -6;
	}
	if (m_status == 0)m_status = 1;
	return 0;
}

int CLocalSocket::Link(CSocketBase** pCliSocket)
{
	if (m_socket == -1 || (m_status < 1))return -1;
	int ret = 0;
	if (m_param.attr & SOCK_ISSERVER) {
		if (pCliSocket == nullptr)return - 2;
		CSockParam param;
		socklen_t len = sizeof(sockaddr_un);
		int fd = accept(m_socket, param.addrun(), &len);
		if (fd == -1)return -3;
		(*pCliSocket) = new CLocalSocket(fd);
		if (*pCliSocket == nullptr)return -4;
		ret=(*pCliSocket)->Init(param);
		if (ret != 0) {
			delete (*pCliSocket);
			pCliSocket = nullptr;
			return -5;
		}
	}
	else {
		//Init给的m_param
		ret = connect(m_socket, m_param.addrun(), sizeof(sockaddr_un));
		if (ret != 0)return -6;
	}
	m_status = 2;
	return 0;
}

int CLocalSocket::Send(const Buffer& buffer)
{
	if (m_status < 2 || (m_socket == -1))return -1;
	ssize_t index = 0;
	while (index < static_cast<ssize_t>(buffer.size())) {
		ssize_t len=write(m_socket, buffer + index, buffer.size() - index);
		if (len == 0) {
			return -2;//连接被关闭
		}
		if (len < 0)return -3; //写操作失败
		index += len;
	}
	return 0;
}

int CLocalSocket::Recv(Buffer& buffer)
{
#ifdef _DEBUG
	{
		std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
		memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
		snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> m_status=%d m_socket=%d\n",
			__FILE__, __LINE__, __FUNCTION__, m_status, m_socket);
		fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
		fflush(pFile);
	}
#endif
	if (m_status < 2 || (m_socket == -1))return -1;
	ssize_t len = read(m_socket, buffer, buffer.size());
	if (len > 0) {
		buffer.resize(len);
		return static_cast<int>(len);
	}
	else if (len < 0) {
		if (errno == EINTR || (errno == EAGAIN)) {
			//没有数据收到
			buffer.clear();
			return 0;
		}
		return -2;//接收错误
	}
	return -3;//连接关闭
}

void CLocalSocket::Close()
{
	CSocketBase::Close();
	return;
}

CLocalSocket::CLocalSocket(int sock)
	:CSocketBase()
{
	m_socket = sock;
}

CLocalSocket::~CLocalSocket()
{
	Close();
}
