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

	//连接
	virtual int Connect(const std::map<Buffer, Buffer>& args) override;
	//执行
	virtual int Exec(const Buffer& sql) override;
	//带结果的执行
	virtual int Exec(const Buffer& sql, std::list<PTable>& result, const _Table_& table) override;
	//开启事务
	virtual int StartTransaction() override;
	//提交事务
	virtual int CommitTransaction() override;
	//回滚事务
	virtual int RollbackTransaction() override;
	//关闭连接
	virtual int Close() override;
	//是否连接
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
	//返回创建的SQL语句
	virtual Buffer TCreate() override;
	//删除表
	virtual Buffer Drop() override;
	//增删改查
	//TODO:参数进行优化
	virtual Buffer Insert(const _Table_& values) override;
	virtual Buffer Delete(const _Table_& values) override;
	//TODO:参数进行优化
	virtual Buffer Modify(const _Table_& values) override;
	virtual Buffer Query(const Buffer& condition = "") override;
	//创建一个基于表的对象
	virtual PTable Copy()const override;
	virtual void ClearFieldUsed();
	//获取表的全名
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
	//查到的结果是字符串，把它转成对应的值
	virtual void LoadFromStr(const Buffer& str) override;
	//where 语句使用的  生成一个=的表达式
	virtual Buffer toEqualExp() const override;
	//转成字符串，表需要列变成字符串
	virtual Buffer toSqlStr() const override;
	//列的全名
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

