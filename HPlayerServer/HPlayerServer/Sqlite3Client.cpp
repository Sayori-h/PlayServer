#include "Sqlite3Client.h"
#include "Logger.h"

int CSqlite3Client::Connect(const std::map<Buffer, Buffer>& args)
{
    auto it = args.find("host");
    if (it == args.end())return -1;
    if (m_db == nullptr)return -2;
    int ret = sqlite3_open(it->second, &m_db);
    if (ret != SQLITE_OK) {
        TRACE_ERROR("connect failed:%d [%s]\n", ret, sqlite3_errmsg(m_db));
        return -3;
    }
    return 0;
}

int CSqlite3Client::Exec(const Buffer& sql)
{
    if (m_db == nullptr)return -1;
    int ret = sqlite3_exec(m_db, sql, nullptr, this, nullptr);
    if (ret != SQLITE_OK) {
        TRACE_ERROR("sql={%s}", sql);
        TRACE_ERROR("exec failed:%d [%s]\n", ret, sqlite3_errmsg(m_db));
        return -2;
    }
    return 0;
}

int CSqlite3Client::Exec(const Buffer& sql, std::list<PTable>& result, const _Table_& table)
{
    char* errmsg = nullptr;
    if (m_db == nullptr)return -1;
    ExecParam param(this, result, table);
    int ret = sqlite3_exec(m_db, sql, &CSqlite3Client::ExecCallback, (void*)&param, &errmsg);
    if (ret != SQLITE_OK) {
        TRACE_ERROR("sql={%s}", sql);
        TRACE_ERROR("exec failed:%d [%s]\n", ret, errmsg);
        if (errmsg)sqlite3_free(errmsg);
        return -2;
    }
    if (errmsg)sqlite3_free(errmsg);
    return 0;
}

int CSqlite3Client::StartTransaction()
{
    if (m_db == nullptr)return -1;
    int ret = sqlite3_exec(m_db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);
    if (ret != SQLITE_OK) {
        TRACE_ERROR("sql={BEGIN TRANSACTION}");
        TRACE_ERROR("BEGIN failed:%d [%s]\n", ret, sqlite3_errmsg(m_db));
        return -2;
    }
    return 0;
}

int CSqlite3Client::CommitTransaction()
{
    if (m_db == nullptr)return -1;
    int ret = sqlite3_exec(m_db, "COMMIT TRANSACTION", nullptr, nullptr, nullptr);
    if (ret != SQLITE_OK) {
        TRACE_ERROR("sql={COMMIT TRANSACTION}");
        TRACE_ERROR("COMMIT failed:%d [%s]\n", ret, sqlite3_errmsg(m_db));
        return -2;
    }
    return 0;
}

int CSqlite3Client::RollbackTransaction()
{
    if (m_db == nullptr)return -1;
    int ret = sqlite3_exec(m_db, "ROLLBACK TRANSACTION", nullptr, nullptr, nullptr);
    if (ret != SQLITE_OK) {
        TRACE_ERROR("sql={ROLLBACK TRANSACTION}");
        TRACE_ERROR("ROLLBACK failed:%d [%s]\n", ret, sqlite3_errmsg(m_db));
        return -2;
    }
    return 0;
}

int CSqlite3Client::Close()
{
    if (m_db == nullptr)return -1;
    int ret = sqlite3_close(m_db);
    if (ret != SQLITE_OK) {
        TRACE_ERROR("Close failed:%d [%s]\n", ret, sqlite3_errmsg(m_db));
        return -2;
    }
    return 0;
}

bool CSqlite3Client::IsConnected()
{
    return m_db!=nullptr;
}

int CSqlite3Client::ExecCallback(void* arg, int count, char** names, char** values)
{
    ExecParam* param = (ExecParam*)arg;
    int res = param->m_obj->ExecCallback(param->m_result, 
        param->m_table, count, names, values);
    return res;
}

int CSqlite3Client::ExecCallback(std::list<PTable>& result, const _Table_& table,
    int count, char** names, char** values)
{
    PTable pTable = table.Copy();
    if (pTable == nullptr) {
        TRACE_ERROR("table %s error!", (const char*)(Buffer)table);
        return -1;
    }
    for (int i = 0; i < count; i++) {
        Buffer name = names[i];
        auto it = pTable->MapFields.find(name);
        if (it == pTable->MapFields.end()) {
            TRACE_ERROR("table %s error!", (const char*)(Buffer)table);
            return -2;
        }
        if (values[i] != nullptr)
            it->second->LoadFromStr(values[i]);
    }
    result.push_back(pTable);
    return 0;
}

_sqlite3_table_::_sqlite3_table_(const _sqlite3_table_& table)
{
    Database = table.Database;
    Name = table.Name;
    for (size_t i = 0; i < table.VecField.size(); i++) {
        //PField pField(new _sqlite3_table_(*(_sqlite3_table_*)table.VecField[i].get()));
        //PField pField = PField(new _sqlite3_field_(*(_sqlite3_field_*)table.VecField[i].get()));
        //发生了隐式向上转型
        PField pField = std::make_shared<_sqlite3_field_>(*(_sqlite3_field_*)table.VecField[i].get());
        VecField.push_back(pField);
        MapFields[pField->Name] = pField;
    }
}

Buffer _sqlite3_table_::Create()
{
    //CREATE TABLE IF NOT EXISTS 表全名 (列定义,……);
    //表全名 = 数据库.表名
    Buffer sql = "CREATE TABLE IF NOT EXISTS" + (Buffer)*this + "(";
    for (size_t i = 0; i < VecField.size(); i++) {
        if (i > 0)sql += ",";
        sql += VecField[i]->Create();
    }
    sql += ");";
    TRACE_INFO("sql=%s", (char*)sql);
    return Buffer();
}

Buffer _sqlite3_table_::Drop()
{
    Buffer sql = "DROP TABLE" + (Buffer)*this + ";";
    TRACE_INFO("sql = %s", (char*)sql);
    return sql;
}

Buffer _sqlite3_table_::Insert(const _Table_& values)
{
    //INSERT INTO TABLE_NAME (column1, column2, column3,...,columnN)
    //VALUES(value1, value2, value3, ...,valueN);
    Buffer sql = "INSERT INTO " + (Buffer)*this + " (";
    bool isfirst = true;
    for (size_t i = 0; i < VecField.size(); i++) {
        if (VecField[i]->Condition & SQL_INSERT) {
            if (!isfirst)sql += ',';
            else isfirst = false;
            sql += (Buffer)*VecField[i];
        }
    }
    sql += ") VALUES(";
    isfirst = true;
    for (size_t i = 0; i < VecField.size(); i++) {
        if (VecField[i]->Condition & SQL_INSERT) {
            if (!isfirst)sql += ',';
            else isfirst = false;
            sql += VecField[i]->toSqlStr();
        }
    }
    sql += " );";
    TRACE_INFO("sql = %s", (char*)sql);
    return sql;
}

Buffer _sqlite3_table_::Delete(const _Table_& values)
{
    //DELETE FROM table_name WHERE condition;
    Buffer sql = "DELETE FROM " + (Buffer)*this + " ";
    Buffer Where = "";
    bool isfirst = true;
    for (size_t i = 0; i < VecField.size(); i++) {
        if (VecField[i]->Condition & SQL_CONDITION) {
            if (!isfirst)Where += " AND ";
            else isfirst = false;
            Where += (Buffer)*VecField[i] + "=" + VecField[i]->toSqlStr();
        }
    }
    if (Where.size() > 0) {
        sql += " WHERE " + Where;
    }
    sql += ';';
    TRACE_INFO("sql = %s", (char*)sql);
    return sql;
}

Buffer _sqlite3_table_::Modify(const _Table_& values)
{
    //UPDATE table_name SET column1 = value1, column2 = value2...., columnN = valueN WHERE condition;
    Buffer sql = "UPDATE " + (Buffer)*this + " (";
    bool isfirst = true;
    for (size_t i = 0; i < VecField.size(); i++) {
        if (VecField[i]->Condition & SQL_MODIFY) {
            if (!isfirst)sql += ",";
            else isfirst = false;
            sql += (Buffer)*VecField[i] + "=" + VecField[i]->toSqlStr();
        }       
    }
    Buffer Where = "";
    for (size_t i = 0; i < VecField.size(); i++) {
        if (VecField[i]->Condition & SQL_CONDITION) {
            if (!isfirst)Where += " AND ";
            else isfirst = false;
            Where += (Buffer)*VecField[i] + "=" + VecField[i]->toSqlStr();
        }
    }
    if (Where.size() > 0)sql += " WHERE " + Where;
    sql += " ;";
    TRACE_INFO("sql = %s", (char*)sql);
    return sql;
}

Buffer _sqlite3_table_::Query()
{
    //SELECT column1, column2, ..., columnN FROM table_name;
    Buffer sql = "SELECT ";
    for (size_t i = 0; i < VecField.size(); i++) {
        if (i > 0)sql += ',';
        sql += '"' + VecField[i]->Name + "\"";
    }
    sql += " FROM " + (Buffer)*this + ";";
    TRACE_INFO("sql = %s", (char*)sql);
    return sql;
}

PTable _sqlite3_table_::Copy() const
{
    PTable pTable=std::make_shared<_sqlite3_table_>(*this);
    return pTable;
}

void _sqlite3_table_::ClearFieldUsed()
{
    for (size_t i = 0; i < VecField.size(); i++) {
        VecField[i]->Condition = 0;
    }
}

_sqlite3_table_::operator const Buffer() const
{
    Buffer Head;
    if (Database.size())
        Head = '"' + Database + "\".";
    return Head + '"' + Name + '"';
}
