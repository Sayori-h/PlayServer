#pragma once
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <fcntl.h>

class Buffer :public std::string
{
public:
	Buffer() :std::string() {}
	Buffer(size_t size) :std::string() { resize(size); }
	Buffer(const std::string& str) :std::string(str){}
	Buffer(const char* str) :std::string(str){}
	operator char* () { return (char*)c_str(); }
	operator char* () const { return (char*)c_str(); }
	//operator const char* () const{ return c_str(); }
	Buffer& operator=(const char* str) {
		resize(strlen(str));
		strcpy((char*)data(), str);
		return *this;
	}
};

enum SockAttr {
	SOCK_ISSERVER = 0b0001,//�Ƿ������ 1��ʾ�� 0��ʾ�ͻ���
	SOCK_ISNONBLOCK = 0b0010,//�Ƿ����� 1��ʾ������ 0��ʾ����
	SOCK_ISUDP = 0b0100,//�Ƿ�ΪUDP 1��ʾudp 0��ʾtcp
};

class CSockParam {
public:
	CSockParam();
	~CSockParam() {}
	CSockParam(const CSockParam& param);
	//�����׽���ͨ�ſ�IP�Ͷ˿�
	CSockParam(const Buffer& ip, short port, int attr);
	//�����׽���ͨ�ſ�·��
	CSockParam(const Buffer& path, int attr);

	CSockParam& operator=(const CSockParam& param);
	sockaddr* addrin() {return (sockaddr*)&addr_in;}
	sockaddr* addrun() {return (sockaddr*)&addr_un;}
public:
	//��ַ,��Ҷ�Ҫʹ�õľͲ��ü�m_
	sockaddr_in addr_in;
	sockaddr_un addr_un;
	//ip
	Buffer ip;
	//�˿�
	short port;
	//�ο�SockAttr
	int attr;
};

class CSocketBase
{
public:
	CSocketBase();
	//ע��������
	virtual ~CSocketBase();
	//��ʼ�� �������׽��ִ�����bind��listen  �ͻ����׽��ִ���
	virtual int Init(const CSockParam& param) = 0;
	//���ӣ����ڷ�����������accept�����ڿͻ�����connect ����udp���Ժ���)
	virtual int Link(CSocketBase** pCliSocket = nullptr) = 0;
	//��������
	virtual int Send(const Buffer& buffer) = 0;
	//��������
	virtual int Recv(Buffer& buffer) = 0;
	//�ر�����
	virtual void Close();
	virtual operator int() { return m_socket; }
	virtual operator int()const { return m_socket; }
protected:
	//�׽�����������Ĭ����-1
	int m_socket;
	//״̬ 0��ʼ��δ��� 1��ʼ����� 2������� 3�Ѿ��ر�
	int m_status;
	//��ʼ������
	CSockParam m_param;
};

class CLocalSocket :public CSocketBase	
{
public:
	//��ʼ��
	virtual int Init(const CSockParam& param) override;
	//���ӣ����ڷ�����������accept�����ڿͻ�����connect����udp���Ժ���
	virtual int Link(CSocketBase** pCliSocket) override;
	//��������
	virtual int Send(const Buffer& buffer) override;
	//��������
	virtual int Recv(Buffer& buffer) override;
	//�ر�����
	virtual void Close() override;
public:
	CLocalSocket() :CSocketBase(){}
	CLocalSocket(int sock);
	virtual ~CLocalSocket();
public:
	CLocalSocket(const CLocalSocket&) = delete;
	CLocalSocket& operator=(const CLocalSocket&) = delete;
};


