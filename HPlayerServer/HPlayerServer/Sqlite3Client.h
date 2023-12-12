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
	virtual int Connect(const std::map<Buffer, Buffer>& args);
	//ִ��
	virtual int Exec(const Buffer& sql);
	//�������ִ��
	virtual int Exec(const Buffer& sql, std::list<PTable>& result, const _Table_& table);
	//��������
	virtual int StartTransaction();
	//�ύ����
	virtual int CommitTransaction();
	//�ع�����
	virtual int RollbackTransaction();
	//�ر�����
	virtual int Close();
	//�Ƿ�����
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

