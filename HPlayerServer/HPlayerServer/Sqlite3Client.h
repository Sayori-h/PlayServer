#pragma once
#include "sqlite3/sqlite3.h"
#include "DatabaseHelper.h"

class CSqlite3Client :
    public CDatabaseClient
{
public:
	CSqlite3Client() {}
	virtual ~CSqlite3Client() {}

	CSqlite3Client(const CSqlite3Client&) = delete;
	CSqlite3Client& operator=(const CSqlite3Client&) = delete;

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
	sqlite3_stmt* m_stmt;
	sqlite3* m_db;

	static int ExecCallback(void* arg, int count, char** names, char** values);
	int ExecCallback(std::list<PTable>& result, const _Table_& table, 
		int count, char** names, char** values);
private:
	class ExecParam {
	public:
		ExecParam(CSqlite3Client* obj, std::list<PTable>& result, const _Table_& table)
			:m_obj(obj),m_result(result),m_table(table)
		{}
		CSqlite3Client* m_obj;
		std::list<PTable>& m_result;
		const _Table_& m_table;
	};
};

class _sqlite3_table_ : public _Table_	
{
public:
	_sqlite3_table_() :_Table_() {}
	_sqlite3_table_(const _sqlite3_table_& table);
	virtual ~_sqlite3_table_() {}
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
	virtual Buffer Query() override;
	//����һ�����ڱ�Ķ���
	virtual PTable Copy()const override;
	virtual void ClearFieldUsed();
	//��ȡ���ȫ��
	virtual operator const Buffer() const override;
};

class _sqlite3_field_ :public _Field_
{
public:
	_sqlite3_field_();
	virtual ~_sqlite3_field_(){}
	_sqlite3_field_(int ntype,const Buffer& name,unsigned attr,
		const Buffer& type,const Buffer& size,
		const Buffer& default_,const Buffer& check);
	_sqlite3_field_(const _sqlite3_field_& field);
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
	struct {
		bool Bool;
		int Integer;
		double Double;
		Buffer String;
	}Value;
	int nType;
};

#define DECLARE_TABLE_CLASS(name, base) class name:public base { \
public: \
virtual PTable Copy() const {return std::make_shared<name>(*this);} \
name():base(){Name=#name;

#define DECLARE_FIELD(ntype,name,attr,type,size,default_,check) \
{PField pField = std::make_shared<_sqlite3_field_>(ntype, #name, attr, type, size, default_, check);VecField.push_back(pField);MapFields[#name] = pField; }

#define DECLARE_TABLE_CLASS_END() }};

