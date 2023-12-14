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
	_sqlite3_table_() :_Table_() {}
	_sqlite3_table_(const _sqlite3_table_& table);
	virtual ~_sqlite3_table_() {}
	//返回创建的SQL语句
	virtual Buffer Create() override;
	//删除表
	virtual Buffer Drop() override;
	//增删改查
	//TODO:参数进行优化
	virtual Buffer Insert(const _Table_& values) override;
	virtual Buffer Delete(const _Table_& values) override;
	//TODO:参数进行优化
	virtual Buffer Modify(const _Table_& values) override;
	virtual Buffer Query() override;
	//创建一个基于表的对象
	virtual PTable Copy()const override;
	virtual void ClearFieldUsed();
public:
	//获取表的全名
	virtual operator const Buffer() const override;
};

class _sqlite3_field_ :public _Field_
{
	virtual Buffer Create() override;
	//查到的结果是字符串，把它转成对应的值
	virtual void LoadFromStr(const Buffer& str) override;
	//where 语句使用的  生成一个=的表达式
	virtual Buffer toEqualExp() const override;
	//转成字符串，表需要列变成字符串
	virtual Buffer toSqlStr() const override;
	//列的全名
	virtual operator const Buffer() const override;
};

