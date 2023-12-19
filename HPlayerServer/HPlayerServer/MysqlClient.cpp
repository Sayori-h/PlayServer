#include "MysqlClient.h"

int CMysqlClient::Connect(const std::map<Buffer, Buffer>& args)
{
    if (m_bInit)return -1;
    MYSQL* ret = mysql_init(&m_db);
    if (ret == nullptr)return -2;
    ret = mysql_real_connect(&m_db, args.at("host"), args.at("user"),
        args.at("password"), args.at("db"), atoi(args.at("port")),nullptr,0);
    if ((ret == nullptr) && (mysql_errno(&m_db)!=0)) {
        printf("%s(%d):<%s> mysql_errno=%d %s\n", __FILE__, __LINE__, 
            __FUNCTION__, mysql_errno(&m_db), mysql_error(&m_db));
        mysql_close(&m_db);
        memset(&m_db, 0, sizeof(m_db));
        return -3;
    }
    m_bInit = true;
    return 0;
}

int CMysqlClient::Exec(const Buffer& sql)
{
    if (!m_bInit)return -1;
    int ret = mysql_real_query(&m_db, sql, sql.size());
    if (ret != 0) {
        printf("%s(%d):<%s> mysql_errno=%d %s\n", __FILE__, __LINE__,
            __FUNCTION__, mysql_errno(&m_db), mysql_error(&m_db));
        return -2;
    }
    return 0;
}

int CMysqlClient::Exec(const Buffer& sql, std::list<PTable>& result, const _Table_& table)
{
    if (!m_bInit)return -1;
    int ret = mysql_real_query(&m_db, sql, sql.size());
    if (ret != 0) {
        printf("%s(%d):<%s> mysql_errno=%d %s\n", __FILE__, __LINE__,
            __FUNCTION__, mysql_errno(&m_db), mysql_error(&m_db));
        return -2;
    }
    MYSQL_RES* res = mysql_store_result(&m_db);
    MYSQL_ROW row;
    unsigned num_fields = mysql_num_fields(res);
    while ((row=mysql_fetch_row(res))!=nullptr) {
        PTable pt = table.Copy();
        for (unsigned i = 0; i < num_fields; i++) 
            if (row[i])
                pt->VecField[i]->LoadFromStr(row[i]);
        result.push_back(pt);
    }
    return 0;
}

int CMysqlClient::StartTransaction()
{
    if (!m_bInit)return -1;
    int ret = mysql_real_query(&m_db, "BEGIN", 6);
    if (ret != 0) {
        printf("%s(%d):<%s> mysql_errno=%d %s\n", __FILE__, __LINE__,
            __FUNCTION__, mysql_errno(&m_db), mysql_error(&m_db));
        return -2;
    }
    return 0;
}

int CMysqlClient::CommitTransaction()
{
    if (!m_bInit)return -1;
    int ret = mysql_real_query(&m_db, "COMMIT", 7);
    if (ret != 0) {
        printf("%s(%d):<%s> mysql_errno=%d %s\n", __FILE__, __LINE__,
            __FUNCTION__, mysql_errno(&m_db), mysql_error(&m_db));
        return -2;
    }
    return 0;
}

int CMysqlClient::RollbackTransaction()
{
    if (!m_bInit)return -1;
    int ret = mysql_real_query(&m_db, "ROLLBACK", 9);
    if (ret != 0) {
        printf("%s(%d):<%s> mysql_errno=%d %s\n", __FILE__, __LINE__,
            __FUNCTION__, mysql_errno(&m_db), mysql_error(&m_db));
        return -2;
    }
    return 0;
}

int CMysqlClient::Close()
{
    if (m_bInit) {
        m_bInit = false;
        mysql_close(&m_db);
        memset(&m_db, 0, sizeof(m_db));
    }
    return 0;
}

bool CMysqlClient::IsConnected()
{
    return m_bInit;
}

_mysql_table_::_mysql_table_(const _mysql_table_& table)
{
    Database = table.Database;
    Name = table.Name;
    for (size_t i = 0; i < table.VecField.size(); i++) {
        PField pField = std::make_shared<_mysql_field_>(*(_mysql_field_*)table.VecField[i].get());
        VecField.push_back(pField);
        MapFields[pField->Name] = pField;
    }
}

Buffer _mysql_table_::TCreate()
{
    /*CREATE TABLE IF NOT EXISTS 表全名 (
    列定义1,
    列定义2,
    ...,
    PRIMARY KEY (`主键列名`),
    UNIQUE INDEX `列名_UNIQUE` (`列名` ASC) VISIBLE );
    */
    Buffer sql = "CREATE TABLE IF NOT EXISTS " + (Buffer)*this + " (\r\n";
    for (size_t i = 0; i < VecField.size(); i++) {
        if (i > 0)sql += ",\r\n";
        sql += VecField[i]->FCreate();
        if (VecField[i]->Attr & PRIMARY_KEY)
            sql += ",\r\n PRIMARY KEY (`" + VecField[i]->Name + "`)";
        if (VecField[i]->Attr & UNIQUE) {
            sql += ",\r\n UNIQUE INDEX `" + VecField[i]->Name + "_UNIQUE` (";
            sql += (Buffer)*VecField[i] + " ASC) VISIBLE ";
        }
    }
    sql += ");";
    return sql;
}

Buffer _mysql_table_::Drop()
{
    return "DROP TABLE"+(Buffer)*this;
}

Buffer _mysql_table_::Insert(const _Table_& values)
{
    //INSERT INTO TABLE_NAME (column1,column2,column3,...,columnN)
    //VALUES(value1,value2,value3, ...,valueN);
    Buffer sql = "INSERT INTO " + (Buffer)*this + " (";
    bool isfirst = true;
    for (size_t i = 0; i < values.VecField.size(); i++) 
        if (values.VecField[i]->Condition & SQL_INSERT) {
            if (!isfirst)sql += ',';
            else isfirst = false;
            sql += (Buffer)*values.VecField[i];
        }
    sql += ") VALUES (";
    isfirst = true;
    for (size_t i = 0; i < values.VecField.size(); i++) 
        if (values.VecField[i]->Condition & SQL_INSERT) {
            if (!isfirst)sql += ',';
            else isfirst = false;
            sql += values.VecField[i]->toSqlStr();
        }
    sql += " );";
    printf("sql = %s", (char*)sql);
    return sql;
}

Buffer _mysql_table_::Delete(const _Table_& values)
{
    //DELETE FROM table_name WHERE condition;
    Buffer sql = "DELETE FROM " + (Buffer)*this + " ";
    Buffer Where = "";
    bool isfirst = true;
    for (size_t i = 0; i < values.VecField.size(); i++) 
        if (values.VecField[i]->Condition & SQL_CONDITION) {
            if (!isfirst)Where += " AND ";
            else isfirst = false;
            Where += (Buffer)*values.VecField[i] + "=" + values.VecField[i]->toSqlStr();
        }
    if (Where.size() > 0) 
        sql += " WHERE " + Where;
    sql += ';';
    printf("sql = %s", (char*)sql);
    return sql;
}

Buffer _mysql_table_::Modify(const _Table_& values)
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
    printf("sql = %s", (char*)sql);
    return sql;
}

Buffer _mysql_table_::Query()
{
    //SELECT column1, column2, ..., columnN FROM table_name;
    Buffer sql = "SELECT ";
    for (size_t i = 0; i < VecField.size(); i++) {
        if (i > 0)sql += ',';
        sql += '`' + VecField[i]->Name + "`";
    }
    sql += " FROM " + (Buffer)*this + ";";
    printf("sql = %s", (char*)sql);
    return sql;
}

PTable _mysql_table_::Copy() const
{
    return std::make_shared<_mysql_table_>(*this);    
}

void _mysql_table_::ClearFieldUsed()
{
    for (size_t i = 0; i < VecField.size(); i++) 
        VecField[i]->Condition = 0;
}

_mysql_table_::operator const Buffer() const
{
    Buffer Head;
    if (Database.size())
        Head = '`' + Database + "`.";
    return Head + '`' + Name + '`';
}

_mysql_field_::_mysql_field_():_Field_()
{
    nType = TYPE_NULL;
    Value.Double = 0.0;
}

_mysql_field_::_mysql_field_(int ntype, const Buffer& name, unsigned attr, const Buffer& type, const Buffer& size, const Buffer& default_, const Buffer& check)
{
    nType = ntype;
    Name = name;
    Attr = attr;
    Type = type;
    Size = size;
    Default = default_;
    Check = check;
}

_mysql_field_::_mysql_field_(const _mysql_field_& field)
    :_Field_(field)
{
    nType = field.nType;
    Value = field.Value;
}

Buffer _mysql_field_::FCreate()
{
    //`名称` 类型 属性
    Buffer sql = '`' + Name + "` " + Type + Size + " ";
    if (Attr & NOT_NULL)sql += " NOT NULL ";
    else sql += " NULL ";
    //BLOB TEXT GEOMETRY（坐标） JSON不能有默认值的
    if ((Attr & DEFAULT) && (!Default.empty()) && (Type != "BLOB")
        && (Type != "TEXT") && (Type != "GEOMETRY") && (Type != "JSON"))
        sql += " DEFAULT " + Default + " ";
    //UNIQUE PRIMARY_KEY 外面处理
    //CHECK mysql不支持
    if (Attr & AUTOINCREMENT) {
        sql += " AUTO_INCREMENT ";
    }
    return sql;
}

void _mysql_field_::LoadFromStr(const Buffer& str)
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
        printf("type=%d", nType);
        break;
    }
}

Buffer _mysql_field_::toEqualExp() const
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
        printf("type=%d", nType);
        break;
    }
    return sql;
}

Buffer _mysql_field_::toSqlStr() const
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
        printf("type=%d", nType);
        break;
    }
    return sql;
}

_mysql_field_::operator const Buffer() const
{
    return '`' + Name + '`';
}

Buffer _mysql_field_::Str2Hex(const Buffer& data) const
{
    const char* hex = "0123456789ABCDEF";
    std::stringstream ss;
    for (char ch : data) {
        ss << hex[(unsigned)ch >> 4] << hex[(unsigned)ch & 0xF];
    }
    return ss.str();
}
