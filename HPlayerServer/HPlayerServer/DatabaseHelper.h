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
	virtual Buffer TCreate() = 0;
	//ɾ����
	virtual Buffer Drop() = 0;
	//��ɾ�Ĳ�
	//TODO:���������Ż�
	virtual Buffer Insert(const _Table_& values) = 0;
	virtual Buffer Delete(const _Table_& values) = 0;
	//TODO:���������Ż�
	virtual Buffer Modify(const _Table_& values) = 0;
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

enum {
	SQL_INSERT = 1,//�������
	SQL_MODIFY = 2,//�޸ĵ���
	SQL_CONDITION = 4//��ѯ������
};

enum {
	NOT_NULL=1,
	DEFAULT=2,
	UNIQUE=4,
	PRIMARY_KEY=8,
	CHECK=16,
	AUTOINCREMENT=32
};

enum SqlType{
	TYPE_NULL = 0,
	TYPE_BOOL = 1,
	TYPE_INT = 2,
	TYPE_DATETIME = 4,
	TYPE_REAL = 8,
	TYPE_VARCHAR = 16,
	TYPE_TEXT = 32,
	TYPE_BLOB = 64
};

class _Field_
{
public:
	_Field_(){}
	_Field_(const _Field_& field);
	virtual _Field_& operator=(const _Field_& field);
	virtual ~_Field_(){}

	virtual Buffer FCreate() = 0;
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
	//��������
	unsigned Condition;
};

#define DECLARE_TABLE_CLASS(name, base) class name:public base { \
public: \
virtual PTable Copy() const {return std::make_shared<name>(*this);} \
name():base(){Name=#name;

#define DECLARE_FIELD(ntype,name,attr,type,size,default_,check) \
{PField pField = std::make_shared<_sqlite3_field_>(ntype, #name, attr, type, size, default_, check);VecField.push_back(pField);MapFields[#name] = pField; }

#define DECLARE_TABLE_CLASS_END() }};