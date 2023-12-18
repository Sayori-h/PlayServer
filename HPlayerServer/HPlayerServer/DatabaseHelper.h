#pragma once
#include "Public.h"
#include <map>
#include <list>
#include <memory>
#include <vector>

class _Table_;
using PTable = std::shared_ptr<_Table_>;

//MySQL连接需要用户名和密码
//using KeyValue = std::map<Buffer, Buffer>;
//using Result = std::list<PTable>;

class CDatabaseClient
{
public:
	CDatabaseClient(){}
	virtual ~CDatabaseClient(){}

	CDatabaseClient(const CDatabaseClient&) = delete;
	CDatabaseClient& operator=(const CDatabaseClient&) = delete;

	//连接
	virtual int Connect(const std::map<Buffer, Buffer>& args) = 0;
	//执行
	virtual int Exec(const Buffer& sql) = 0;
	//带结果的执行
	virtual int Exec(const Buffer& sql, std::list<PTable>& result, const _Table_& table) = 0;
	//开启事务
	virtual int StartTransaction() = 0;
	//提交事务
	virtual int CommitTransaction() = 0;
	//回滚事务
	virtual int RollbackTransaction() = 0;
	//关闭连接
	virtual int Close() = 0;
	//是否连接
	virtual bool IsConnected() = 0;
};

//表和列的基类的实现

class _Field_;
using PField = std::shared_ptr<_Field_>;

class _Table_
{
public:
	_Table_(){}
	virtual ~_Table_(){}

	//返回创建表的SQL语句
	//并不是说直接创建表，创建表一定要在数据库客户端CDatabaseClient里去做
	//要想对数据进行操作，只能通过创建语句，通过CDatabaseClient::exec去执行,最后得到一个结果
	virtual Buffer TCreate() = 0;
	//删除表
	virtual Buffer Drop() = 0;
	//增删改查
	//TODO:参数进行优化
	virtual Buffer Insert(const _Table_& values) = 0;
	virtual Buffer Delete(const _Table_& values) = 0;
	//TODO:参数进行优化
	virtual Buffer Modify(const _Table_& values) = 0;
	virtual Buffer Query() = 0;
	//创建一个基于表的对象
	/*传一个对象进去，根据传的对象创建一个副本，这个东西主要给查询用的，exec中会传一个table进去，
	这个对应的就是结果集表的数据，Result里面就是一堆表的数据，每一条结果会转成一个table，
	因为只能传一个table，这个时候，我需要不断创建这个对象，但是我传的是个基类，到时候_Table_还会派生为
	sqlite Table和MySQL Table，但是基类不知道是什么类型，只有子类知道，所以只能公布一个接口，
	如果是sqlite表，copy时就创建一个sqlite对象，但父类还是Table，对外还是这些接口，
	实际根据需求决定是什么类型表，通过这个保证内外一致性*/
	virtual PTable Copy() const = 0;
	//获取表的全名，不同数据库表的全名有差异，最好各做各的
	virtual operator const Buffer()const = 0;
public:
	//表的属性
	//表所属的DB的名称
	Buffer Database;
	Buffer Name;
	//列的定义（存储查询结果）有序的
	std::vector<PField> VecField;
	//列的定义映射表 方便查询，直接通过find去查
	std::map<Buffer, PField> MapFields;
};

enum {
	SQL_INSERT = 1,//插入的列
	SQL_MODIFY = 2,//修改的列
	SQL_CONDITION = 4//查询条件列
};

enum {
	NOT_NULL=1,
	DEFAULT=2,
	UNIQUE=4,
	PRIMARY_KEY=8,
	CHECK=16,
	AUTOINCREMENT=32
};

enum SqlType{
	TYPE_NULL = 0,
	TYPE_BOOL = 1,
	TYPE_INT = 2,
	TYPE_DATETIME = 4,
	TYPE_REAL = 8,
	TYPE_VARCHAR = 16,
	TYPE_TEXT = 32,
	TYPE_BLOB = 64
};

class _Field_
{
public:
	_Field_(){}
	_Field_(const _Field_& field);
	virtual _Field_& operator=(const _Field_& field);
	virtual ~_Field_(){}

	virtual Buffer FCreate() = 0;
	//查到的结果是字符串，把它转成对应的值
	virtual void LoadFromStr(const Buffer& str) = 0;
	//where 语句使用的  生成一个=的表达式
	virtual Buffer toEqualExp() const = 0;
	//转成字符串，表需要列变成字符串
	virtual Buffer toSqlStr() const = 0;
	//列的全名
	virtual operator const Buffer() const = 0;
public:
	Buffer Name;
	Buffer Type;
	//TINYTEXT(255)
	Buffer Size;
	//属性：唯一性，主键，非空
	unsigned Attr;
	//默认值
	Buffer Default;
	//约束条件
	Buffer Check;
	//操作条件
	unsigned Condition;
};

#define DECLARE_TABLE_CLASS(name, base) class name:public base { \
public: \
virtual PTable Copy() const {return std::make_shared<name>(*this);} \
name():base(){Name=#name;

#define DECLARE_FIELD(ntype,name,attr,type,size,default_,check) \
{PField pField = std::make_shared<_sqlite3_field_>(ntype, #name, attr, type, size, default_, check);VecField.push_back(pField);MapFields[#name] = pField; }

#define DECLARE_TABLE_CLASS_END() }};