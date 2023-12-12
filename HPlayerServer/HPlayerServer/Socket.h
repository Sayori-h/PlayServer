#pragma once
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <fcntl.h>
#include <cstring>
#include "Public.h"

enum SockAttr {
	SOCK_ISSERVER = 0b0001,//是否服务器 1表示是 0表示客户端
	SOCK_ISNONBLOCK = 0b0010,//是否阻塞 1表示非阻塞 0表示阻塞
	SOCK_ISUDP = 0b0100,//是否为UDP 1表示udp 0表示tcp
	SOCK_ISIP=0b1000,//是否为IP协议 1表示IP协议 0表示本地套接字
};

class CSockParam {
public:
	CSockParam();
	~CSockParam() {}
	CSockParam(const CSockParam& param);
	//网络套接字通信靠IP和端口
	CSockParam(const Buffer& ip, short port, int attr);
	//本地套接字通信靠路径
	CSockParam(const Buffer& path, int attr);
	//客户端地址传递
	CSockParam(const sockaddr_in* addrin, int attr);

	CSockParam& operator=(const CSockParam& param);
	sockaddr* addrin() {return (sockaddr*)&addr_in;}
	sockaddr* addrun() {return (sockaddr*)&addr_un;}
public:
	//地址,大家都要使用的就不用加m_
	sockaddr_in addr_in;
	sockaddr_un addr_un;
	//ip
	Buffer ip;
	//端口
	short port;
	//参考SockAttr
	int attr;
};

class CSocketBase
{
public:
	CSocketBase();
	//注意虚析构
	virtual ~CSocketBase();
	//初始化 服务器套接字创建、bind、listen  客户端套接字创建
	virtual int Init(const CSockParam& param) = 0;
	//连接（对于服务器，等于accept，对于客户端是connect 对于udp可以忽略)
	virtual int Link(CSocketBase** pCliSocket = nullptr) = 0;
	//发送数据
	virtual int Send(const Buffer& buffer) = 0;
	//接收数据
	virtual int Recv(Buffer& buffer) = 0;
	//关闭连接
	virtual int Close();
	virtual operator int() { return m_socket; }
	virtual operator int()const { return m_socket; }
	virtual operator sockaddr_in* () { return &m_param.addr_in; }
	virtual operator const sockaddr_in* ()const { return &m_param.addr_in; }
protected:
	//套接字描述符，默认是-1
	int m_socket;
	//状态 0初始化未完成 1初始化完成 2连接完成 3已经关闭
	int m_status;
	//初始化参数
	CSockParam m_param;
};

class CSocket :public CSocketBase	
{
public:
	//初始化
	virtual int Init(const CSockParam& param) override;
	//连接（对于服务器，等于accept，对于客户端是connect对于udp可以忽略
	virtual int Link(CSocketBase** pCliSocket=nullptr) override;
	//发送数据
	virtual int Send(const Buffer& buffer) override;
	//接收数据
	virtual int Recv(Buffer& buffer) override;
	//关闭连接
	virtual int Close() override;
public:
	CSocket() :CSocketBase(){}
	CSocket(int sock);
	virtual ~CSocket();
public:
	CSocket(const CSocket&) = delete;
	CSocket& operator=(const CSocket&) = delete;
};


