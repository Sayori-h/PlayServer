#pragma once
#include "Public.h"
#include <map>
#include <list>
#include <memory>
#include <vector>

class _Table_;
using PTable = std::shared_ptr<_Table_>;

//MySQL������Ҫ�û���������
//using KeyValue = std::map<Buffer, Buffer>;
//using Result = std::list<PTable>;

class CDatabaseClient
{
public:
	CDatabaseClient(){}
	virtual ~CDatabaseClient(){}

	CDatabaseClient(const CDatabaseClient&) = delete;
	CDatabaseClient& operator=(const CDatabaseClient&) = delete;

	//����
	virtual int Connect(const std::map<Buffer, Buffer>& args) = 0;
	//ִ��
	virtual int Exec(const Buffer& sql) = 0;
	//�������ִ��
	virtual int Exec(const Buffer& sql, std::list<PTable>& result, const _Table_& table) = 0;
	//��������
	virtual int StartTransaction() = 0;
	//�ύ����
	virtual int CommitTransaction() = 0;
	//�ع�����
	virtual int RollbackTransaction() = 0;
	//�ر�����
	virtual int Close() = 0;
	//�Ƿ�����
	virtual bool IsConnected() = 0;
};

//����еĻ����ʵ��

class _Field_;
using PField = std::shared_ptr<_Field_>;

class _Table_
{
public:
	_Table_(){}
	virtual ~_Table_(){}

	//���ش������SQL���
	//������˵ֱ�Ӵ�����������һ��Ҫ�����ݿ�ͻ���CDatabaseClient��ȥ��
	//Ҫ������ݽ��в�����ֻ��ͨ��������䣬ͨ��CDatabaseClient::execȥִ��,���õ�һ�����
	virtual Buffer Create() = 0;
	//ɾ����
	virtual Buffer Drop() = 0;
	//��ɾ�Ĳ�
	//TODO:���������Ż�
	virtual Buffer Insert(const _Table_& values) = 0;
	virtual Buffer Delete() = 0;
	//TODO:���������Ż�
	virtual Buffer Modify() = 0;
	virtual Buffer Query() = 0;
	//����һ�����ڱ�Ķ���
	/*��һ�������ȥ�����ݴ��Ķ��󴴽�һ�����������������Ҫ����ѯ�õģ�exec�лᴫһ��table��ȥ��
	�����Ӧ�ľ��ǽ����������ݣ�Result�������һ�ѱ�����ݣ�ÿһ�������ת��һ��table��
	��Ϊֻ�ܴ�һ��table�����ʱ������Ҫ���ϴ���������󣬵����Ҵ����Ǹ����࣬��ʱ��_Table_��������Ϊ
	sqlite Table��MySQL Table�����ǻ��಻֪����ʲô���ͣ�ֻ������֪��������ֻ�ܹ���һ���ӿڣ�
	�����sqlite��copyʱ�ʹ���һ��sqlite���󣬵����໹��Table�����⻹����Щ�ӿڣ�
	ʵ�ʸ������������ʲô���ͱ�ͨ�������֤����һ����*/
	virtual PTable Copy() const = 0;
	//��ȡ���ȫ������ͬ���ݿ���ȫ���в��죬��ø�������
	virtual operator const Buffer()const = 0;
public:
	//�������
	//��������DB������
	Buffer Database;
	Buffer Name;
	//�еĶ��壨�洢��ѯ����������
	std::vector<PField> VecField;
	//�еĶ���ӳ��� �����ѯ��ֱ��ͨ��findȥ��
	std::map<Buffer, PField> MapFields;
};

class _Field_
{
public:
	_Field_(){}
	_Field_(const _Field_& field);
	virtual _Field_& operator=(const _Field_& field);
	virtual ~_Field_(){}

	virtual Buffer Create() = 0;
	//�鵽�Ľ�����ַ���������ת�ɶ�Ӧ��ֵ
	virtual void LoadFromStr(const Buffer& str) = 0;
	//where ���ʹ�õ�  ����һ��=�ı��ʽ
	virtual Buffer toEqualExp() const = 0;
	//ת���ַ���������Ҫ�б���ַ���
	virtual Buffer toSqlStr() const = 0;
	//�е�ȫ��
	virtual operator const Buffer() const = 0;
public:
	Buffer Name;
	Buffer Type;
	//TINYTEXT(255)
	Buffer Size;
	//���ԣ�Ψһ�ԣ��������ǿ�
	unsigned Attr;
	//Ĭ��ֵ
	Buffer Default;
	//Լ������
	Buffer Check;
};
