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
	virtual int Connect(const std::map<Buffer, Buffer>& args);
	//执行
	virtual int Exec(const Buffer& sql);
	//带结果的执行
	virtual int Exec(const Buffer& sql, std::list<PTable>& result, const _Table_& table);
	//开启事务
	virtual int StartTransaction();
	//提交事务
	virtual int CommitTransaction();
	//回滚事务
	virtual int RollbackTransaction();
	//关闭连接
	virtual int Close();
	//是否连接
	virtual bool IsConnected();
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

