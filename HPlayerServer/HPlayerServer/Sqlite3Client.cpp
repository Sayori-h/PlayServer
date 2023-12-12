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
        TRACE_ERROR("connect failed:%d [%s]\n", ret, sqlite3_errmsg(m_db));
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
        TRACE_ERROR("connect failed:%d [%s]\n", ret, sqlite3_errmsg(m_db));
        return -2;
    }
    return 0;
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
