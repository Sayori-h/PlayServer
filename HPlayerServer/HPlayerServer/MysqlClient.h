#pragma once
#include <sstream>
#include <mysql/mysql.h>
#include "DatabaseHelper.h"

class CMysqlClient :
    public CDatabaseClient
{
public:
	CMysqlClient() {}
	virtual ~CMysqlClient() {}

	CMysqlClient(const CMysqlClient&) = delete;
	CMysqlClient& operator=(const CMysqlClient&) = delete;

	//����
	virtual int Connect(const std::map<Buffer, Buffer>& args) override;
	//ִ��
	virtual int Exec(const Buffer& sql) override;
	//�������ִ��
	virtual int Exec(const Buffer& sql, std::list<PTable>& result, const _Table_& table) override;
	//��������
	virtual int StartTransaction() override;
	//�ύ����
	virtual int CommitTransaction() override;
	//�ع�����
	virtual int RollbackTransaction() override;
	//�ر�����
	virtual int Close() override;
	//�Ƿ�����
	virtual bool IsConnected() override;
private:
	MYSQL m_db;
	bool m_bInit;
private:
	class ExecParam {
	public:
		ExecParam(CMysqlClient* obj, std::list<PTable>& result, const _Table_& table)
			:m_obj(obj), m_result(result), m_table(table)
		{}
		CMysqlClient* m_obj;
		std::list<PTable>& m_result;
		const _Table_& m_table;
	};
};

class _mysql_table_ : public _Table_
{
public:
	_mysql_table_() :_Table_() {}
	_mysql_table_(const _mysql_table_& table);
	virtual ~_mysql_table_() {}
	//���ش�����SQL���
	virtual Buffer TCreate() override;
	//ɾ����
	virtual Buffer Drop() override;
	//��ɾ�Ĳ�
	//TODO:���������Ż�
	virtual Buffer Insert(const _Table_& values) override;
	virtual Buffer Delete(const _Table_& values) override;
	//TODO:���������Ż�
	virtual Buffer Modify(const _Table_& values) override;
	virtual Buffer Query(const Buffer& condition = "") override;
	//����һ�����ڱ�Ķ���
	virtual PTable Copy()const override;
	virtual void ClearFieldUsed();
	//��ȡ���ȫ��
	virtual operator const Buffer() const override;
};

class _mysql_field_ :public _Field_
{
public:
	_mysql_field_();
	virtual ~_mysql_field_() {}
	_mysql_field_(int ntype, const Buffer& name, unsigned attr,
		const Buffer& type, const Buffer& size,
		const Buffer& default_, const Buffer& check);
	_mysql_field_(const _mysql_field_& field);
	virtual Buffer FCreate() override;
	//�鵽�Ľ�����ַ���������ת�ɶ�Ӧ��ֵ
	virtual void LoadFromStr(const Buffer& str) override;
	//where ���ʹ�õ�  ����һ��=�ı��ʽ
	virtual Buffer toEqualExp() const override;
	//ת���ַ���������Ҫ�б���ַ���
	virtual Buffer toSqlStr() const override;
	//�е�ȫ��
	virtual operator const Buffer() const override;
private:
	Buffer Str2Hex(const Buffer& data)const;
};

#define DECLARE_TABLE_CLASS(name, base) class name:public base { \
public: \
virtual PTable Copy() const {return std::make_shared<name>(*this);} \
name():base(){Name=#name;

#define DECLARE_MYSQL_FIELD(ntype,name,attr,type,size,default_,check) \
{PField pField = std::make_shared<_mysql_field_>(ntype, #name, attr, type, size, default_, check);VecField.push_back(pField);MapFields[#name] = pField; }

#define DECLARE_TABLE_CLASS_END() }};

