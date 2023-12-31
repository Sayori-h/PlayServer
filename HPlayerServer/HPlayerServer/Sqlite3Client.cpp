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
        printf("sql={%s}\n", (char*)sql);
        printf("exec failed:%d [%s]\n", ret, sqlite3_errmsg(m_db));
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
        printf("sql={%s}\n", (char*)sql);
        printf("exec failed:%d [%s]\n", ret, errmsg);
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

int CSqlite3Client::ExecCallback(void* arg, int count, char** values, char** names)
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
        printf("table %s error!\n", (const char*)(Buffer)table);
        return -1;
    }
    for (int i = 0; i < count; i++) {
        Buffer name = names[i];
        auto it = pTable->MapFields.find(name);
        if (it == pTable->MapFields.end()) {
            printf("table %s error!\n", (const char*)(Buffer)table);
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

Buffer _sqlite3_table_::TCreate()
{
    //CREATE TABLE IF NOT EXISTS 表全名 (列定义,……);
    //表全名 = 数据库.表名
    Buffer sql = "CREATE TABLE IF NOT EXISTS " + (Buffer)*this + "(\r\n";
    for (size_t i = 0; i < VecField.size(); i++) {
        if (i > 0)sql += ",";
        sql += VecField[i]->FCreate();
    }
    sql += ");";
    TRACE_INFO("sql=%s", (char*)sql);
    return sql;
}

Buffer _sqlite3_table_::Drop()
{
    Buffer sql = "DROP TABLE" + (Buffer)*this + ";";
    TRACE_INFO("sql = %s", (char*)sql);
    return sql;
}

Buffer _sqlite3_table_::Insert(const _Table_& values)
{
    //INSERT INTO TABLE_NAME (column1,column2,column3,...,columnN)
    //VALUES(value1,value2,value3, ...,valueN);
    Buffer sql = "INSERT INTO " + (Buffer)*this + " (";
    bool isfirst = true;
    for (size_t i = 0; i < values.VecField.size(); i++) {
        if (values.VecField[i]->Condition & SQL_INSERT) {
            if (!isfirst)sql += ',';
            else isfirst = false;
            sql += (Buffer)*values.VecField[i];
        }
    }
    sql += ") VALUES (";
    isfirst = true;
    for (size_t i = 0; i < values.VecField.size(); i++) {
        if (values.VecField[i]->Condition & SQL_INSERT) {
            if (!isfirst)sql += ',';
            else isfirst = false;
            sql += values.VecField[i]->toSqlStr();
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
    for (size_t i = 0; i < values.VecField.size(); i++) {
        if (values.VecField[i]->Condition & SQL_CONDITION) {
            if (!isfirst)Where += " AND ";
            else isfirst = false;
            Where += (Buffer)*values.VecField[i] + "=" + values.VecField[i]->toSqlStr();
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
    Buffer sql = "UPDATE " + (Buffer)*this + " SET ";
    bool isfirst = true;
    for (size_t i = 0; i < values.VecField.size(); i++) {
        if (values.VecField[i]->Condition & SQL_MODIFY) {
            if (!isfirst)sql += ",";
            else isfirst = false;
            sql += (Buffer)*values.VecField[i] + "=" + values.VecField[i]->toSqlStr();
        }       
    }
    Buffer Where = "";
    for (size_t i = 0; i < values.VecField.size(); i++) {
        if (values.VecField[i]->Condition & SQL_CONDITION) {
            if (!isfirst)Where += " AND ";
            else isfirst = false;
            Where += (Buffer)*values.VecField[i] + "=" + values.VecField[i]->toSqlStr();
        }
    }
    if (Where.size() > 0)sql += " WHERE " + Where;
    sql += " ;";
    TRACE_INFO("sql = %s", (char*)sql);
    return sql;
}

Buffer _sqlite3_table_::Query(const Buffer& condition)
{
    //SELECT column1, column2, ..., columnN FROM table_name;
    Buffer sql = "SELECT ";
    for (size_t i = 0; i < VecField.size(); i++) {
        if (i > 0)sql += ',';
        sql += '"' + VecField[i]->Name + "\"";
    }
    sql += " FROM " + (Buffer)*this + " ";
    if (condition.size() > 0)
        sql += " WHERE " + condition;
    sql += ";";
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

_sqlite3_field_::_sqlite3_field_():_Field_()
{
    nType = TYPE_NULL;
    Value.Double = 0.0;
}

_sqlite3_field_::_sqlite3_field_(int ntype, const Buffer& name, unsigned attr, 
    const Buffer& type, const Buffer& size, const Buffer& default_, const Buffer& check)
{
    nType = ntype;
    Name = name;
    Attr = attr;
    Type = type;
    Size = size;
    Default = default_;
    Check = check;
}

_sqlite3_field_::_sqlite3_field_(const _sqlite3_field_& field)
    :_Field_(field)
{
    nType = field.nType;
    Value = field.Value;
}

Buffer _sqlite3_field_::FCreate()
{
    //"名称" 类型 属性
    Buffer sql = '"' + Name + "\" " + Type + " ";
    if (Attr & NOT_NULL) {
        sql += " NOT NULL ";
    }
    if (Attr & DEFAULT) {
        sql += " DEFAULT " + Default + " ";
    }
    if (Attr & UNIQUE) {
        sql += " UNIQUE ";
    }
    if (Attr & PRIMARY_KEY) {
        sql+=" PRIMARY KEY ";
    }
    if (Attr & CHECK) {
        sql += " CHECK( " + Check + ") ";
    }
    if (Attr & AUTOINCREMENT) {
        sql += " AUTOINCREMENT ";
    }
    return sql;
}

void _sqlite3_field_::LoadFromStr(const Buffer& str)
{
    switch (nType)
    {
    case TYPE_NULL:
        break;
    case TYPE_BOOL:
    case TYPE_INT:
    case TYPE_DATETIME:
        Value.Integer = atoi(str);
        break;
    case TYPE_REAL:
        Value.Double = atof(str);
        break;
    case TYPE_VARCHAR:
    case TYPE_TEXT:
        Value.String = str;
        break;
    case TYPE_BLOB:
        Value.String = Str2Hex(str);
        break;
    default:
        TRACE_WARNING("type=%d", nType);
        break;
    }
}

Buffer _sqlite3_field_::toEqualExp() const
{
    Buffer sql = (Buffer)*this + " = ";
    std::stringstream ss;
    switch (nType)
    {
    case TYPE_NULL:
        sql += " NULL ";
        break;
    case TYPE_BOOL:
    case TYPE_INT:
    case TYPE_DATETIME:
        ss << Value.Integer;
        sql += ss.str() + " ";
        break;
    case TYPE_REAL:
        ss << Value.Double;
        sql += ss.str() + " ";
        break;
    case TYPE_VARCHAR:
    case TYPE_TEXT:
    case TYPE_BLOB:
        sql += '"' + Value.String + "\" ";
        break;
    default:
        TRACE_WARNING("type=%d", nType);
        break;
    }
    return sql;
}

Buffer _sqlite3_field_::toSqlStr() const
{
    Buffer sql = "";
    std::stringstream ss;
    switch (nType)
    {
    case TYPE_NULL:
        sql += " NULL ";
        break;
    case TYPE_BOOL:
    case TYPE_INT:
    case TYPE_DATETIME:
        ss << Value.Integer;
        sql += ss.str() + " ";
        break;
    case TYPE_REAL:
        ss << Value.Double;
        sql += ss.str() + " ";
        break;
    case TYPE_VARCHAR:
    case TYPE_TEXT:
    case TYPE_BLOB:
        sql += '"' + Value.String + "\" ";
        break;
    default:
        TRACE_WARNING("type=%d", nType);
        break;
    }
    return sql;
}

_sqlite3_field_::operator const Buffer() const
{
    return '"' + Name + '"';
}

Buffer _sqlite3_field_::Str2Hex(const Buffer& data) const
{
    const char* hex = "0123456789ABCDEF";
    std::stringstream ss;
    for (char ch : data) {
        ss << hex[(unsigned)ch >> 4] << hex[(unsigned)ch & 0xF];
    }
    return ss.str();
}
