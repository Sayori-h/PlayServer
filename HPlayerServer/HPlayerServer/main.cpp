#include "HPlayServer.h"
#include "HttpParser.h"

int CreateLogServer(CProcess* proc) {
    CLoggerServer server;
    int ret=server.Start();//开启日志服务器
    if (ret != 0) {
#ifdef _DEBUG
        {
            std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
            memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
            snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> pid=%d errno:%d msg:%s ret:%d\n",
                __FILE__, __LINE__, __FUNCTION__, getpid(), errno, strerror(errno), ret);
            fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
            fflush(pFile);
        }
#endif // DEBUG 
    }
    int fd = 0;
    while (true) {
        ret=proc->RecvFD(fd);//等待主进程发送终止信号
#ifdef _DEBUG
        {
            std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
            memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
            snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> ret=%d fd=%d\n",
                __FILE__, __LINE__, __FUNCTION__, ret, fd);
            fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
            fflush(pFile);
        }
#endif // DEBUG 
        if (fd <=0)break;
    }
    ret=server.Close();
#ifdef _DEBUG
    {
        std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
        memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
        snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> ret=%d\n",
            __FILE__, __LINE__, __FUNCTION__, ret);
        fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
        fflush(pFile);
    }
#endif // DEBUG 
    return 0;
}

int CreateClientServer(CProcess* proc) {
#ifdef DEBUG
    {
        std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
        memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
        snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> pid=%d\n", __FILE__, __LINE__, __FUNCTION__, getpid());
        fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
        fflush(pFile);
    }
#endif // DEBUG  
    int fd = -1;
    /*int ret = */proc->RecvFD(fd);
#ifdef DEBUG
    {
        std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
        memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
        snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);
        snprintf(szBufInfo + strlen(szBufInfo), BUF_SIZE - strlen(szBufInfo),
            "%s(%d):<%s> fd=%d\n", __FILE__, __LINE__, __FUNCTION__, fd);
        fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
        fflush(pFile);
    }
#endif // DEBUG  
    sleep(1);
    char buf[20] = "";
    lseek(fd, 0, SEEK_SET);
    read(fd, buf, sizeof(buf));
#ifdef DEBUG
    {
        std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
        memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
        snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> buf=%s\n", __FILE__, __LINE__, __FUNCTION__, buf);
        fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
        fflush(pFile);
    }
#endif // DEBUG 
    close(fd);
    return 0;
}

int LogTest()
{
    char buffer[] = "hello world! 恕瑞玛";
    usleep(1000 * 100);
    TRACE_INFO("here is log %d %c %f %g %s 哈哈 日志第一条", 10, 'A', 1.0f, 2.0, buffer);
    DUMP_DEBUG((void*)buffer, (size_t)sizeof(buffer));
    LOG_ERROR << 100 << " " << 'S' << " " << 0.12345f
        << " " << 1.23456789 << " " << buffer << " 日志最后一条";
    return 0;
}

int OldTest() {
    //CProcess::SwitchDeamon();
    CProcess proclog, proclients;
#ifdef _DEBUG
    {
        std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
        memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
        snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> pid=%d\n",
            __FILE__, __LINE__, __FUNCTION__, getpid());
        fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
        fflush(pFile);
    }
#endif // DEBUG  
    proclog.SetEntryFunction(CreateLogServer, &proclog);
    int ret = proclog.CreateSubProcess();
    if (ret) {
#ifdef _DEBUG
        {
            std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
            memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
            snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> pid=%d\n",
                __FILE__, __LINE__, __FUNCTION__, getpid());
            fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
            fflush(pFile);
        }
#endif // DEBUG  
        return -1;
    }

    LogTest();

#ifdef _DEBUG
    {
        std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
        memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
        snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> pid=%d\n",
            __FILE__, __LINE__, __FUNCTION__, getpid());
        fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
        fflush(pFile);
    }
#endif // DEBUG  

    CThread thread(LogTest);
    thread.Start();

    proclients.SetEntryFunction(CreateClientServer, &proclients);
    ret = proclients.CreateSubProcess();
    if (ret) {
#ifdef _DEBUG
        {
            std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
            memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
            snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> pid=%d\n",
                __FILE__, __LINE__, __FUNCTION__, getpid());
            fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
            fflush(pFile);
        }
#endif // DEBUG  
        return -2;
    }
#ifdef _DEBUG
    {
        std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
        memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
        snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> pid=%d\n",
            __FILE__, __LINE__, __FUNCTION__, getpid());
        fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
        fflush(pFile);
    }
#endif // DEBUG  
    sleep(1);
    int fd = open("./testFD.txt", O_RDWR | O_CREAT | O_APPEND);
#ifdef _DEBUG
    {
        std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
        memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
        snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> fd=%d\n",
            __FILE__, __LINE__, __FUNCTION__, fd);
        fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
        fflush(pFile);
    }
#endif // DEBUG  
    if (fd == -1)return -3;
    ret = proclients.SendFD(fd);
    if (ret != 0) {
#ifdef _DEBUG
        {
            std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
            memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
            snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> ret=%d\n",
                __FILE__, __LINE__, __FUNCTION__, ret);
            snprintf(szBufInfo + strlen(szBufInfo), BUF_SIZE - strlen(szBufInfo),
                "errno:%d msg:%s\n", errno, strerror(errno));
            fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
            fflush(pFile);
        }
#endif // DEBUG  
        return -4;
    }
    write(fd, "Fd Send Test...\n", 16);
    close(fd);

    CThreadPool pool;
    ret = pool.Start(4);
    if (ret != 0) {
#ifdef _DEBUG
        {
            std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
            memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
            snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> ret=%d\n",
                __FILE__, __LINE__, __FUNCTION__, ret);
            snprintf(szBufInfo + strlen(szBufInfo), BUF_SIZE - strlen(szBufInfo),
                "errno:%d msg:%s\n", errno, strerror(errno));
            fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
            fflush(pFile);
        }
#endif // DEBUG  
    }
    ret = pool.AddTask(LogTest);
    if (ret != 0) {
#ifdef _DEBUG
        {
            std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
            memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
            snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> ret=%d\n",
                __FILE__, __LINE__, __FUNCTION__, ret);
            snprintf(szBufInfo + strlen(szBufInfo), BUF_SIZE - strlen(szBufInfo),
                "errno:%d msg:%s\n", errno, strerror(errno));
            fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
            fflush(pFile);
        }
#endif // DEBUG  
    }
    ret = pool.AddTask(LogTest);
    if (ret != 0) {
#ifdef _DEBUG
        {
            std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
            memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
            snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> ret=%d\n",
                __FILE__, __LINE__, __FUNCTION__, ret);
            snprintf(szBufInfo + strlen(szBufInfo), BUF_SIZE - strlen(szBufInfo),
                "errno:%d msg:%s\n", errno, strerror(errno));
            fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
            fflush(pFile);
        }
#endif // DEBUG  
    }
    ret = pool.AddTask(LogTest);
    if (ret != 0) {
#ifdef _DEBUG
        {
            std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
            memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
            snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> ret=%d\n",
                __FILE__, __LINE__, __FUNCTION__, ret);
            snprintf(szBufInfo + strlen(szBufInfo), BUF_SIZE - strlen(szBufInfo),
                "errno:%d msg:%s\n", errno, strerror(errno));
            fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
            fflush(pFile);
        }
#endif // DEBUG  
    }
    ret = pool.AddTask(LogTest);
    if (ret != 0) {
#ifdef _DEBUG
        {
            std::lock_guard<std::mutex> lock(debugMutex); // Lock the mutex
            memset(szBufInfo, 0, sizeof(szBufInfo));  // 清零缓冲区
            snprintf(szBufInfo, BUF_SIZE, "%s(%d):<%s> ret=%d\n",
                __FILE__, __LINE__, __FUNCTION__, ret);
            snprintf(szBufInfo + strlen(szBufInfo), BUF_SIZE - strlen(szBufInfo),
                "errno:%d msg:%s\n", errno, strerror(errno));
            fwrite(szBufInfo, sizeof(char), sizeof(szBufInfo), pFile);
            fflush(pFile);
        }
#endif // DEBUG  
    }

    fclose(pFile);
    (void)getchar();
    pool.Close();
    proclog.SendFD(-1);
    (void)getchar();
    return 0;
}

int BussinessTest()
{
    int ret = 0;
    CProcess proclog;
    ret = proclog.SetEntryFunction(CreateLogServer, &proclog);
    ERR_RETURN(ret, -1);
    ret = proclog.CreateSubProcess();
    ERR_RETURN(ret, -2);
    CHPlayServer business(4);
    CServer server;
    ret = server.Init(&business);
    ERR_RETURN(ret, -3);
    ret = server.Run();
    ERR_RETURN(ret, -4);
    return 0;
}

int http_test() {
    Buffer str = "GET /favicon.ico HTTP/1.1\r\n"
        "Host: 0.0.0.0=5000\r\n"
        "User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9) Gecko/2008061015 Firefox/3.0\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*; q = 0.8\r\n"
        "Accept-Language: en-us,en;q=0.5\r\n"
        "Accept-Encoding: gzip,deflate\r\n"
        "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
        "Keep-Alive: 300\r\n"
        "Connection: keep-alive\r\n"
        "\r\n";
    CHttpParser parser;
    size_t size=parser.ExecParser(str);
    if (parser.Errno() != 0) {
        printf("%s(%d):<%s> errno=%d\n",
            __FILE__, __LINE__, __FUNCTION__, parser.Errno());
        return -1;
    }
    if (size != 368) {
        printf("%s(%d):<%s> size error:%lld<->%lld\n",
            __FILE__, __LINE__, __FUNCTION__, size, str.size());
        return -2;
    }
    printf("%s(%d):<%s> method=%d url=%s\n",
        __FILE__, __LINE__, __FUNCTION__, parser.Method(), (char*)parser.Url());
    //错误测试用例
    str = "GET /favicon.ico HTTP/1.1\r\n"
        "Host: 0.0.0.0=5000\r\n"
        "User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9) Gecko/2008061015 Firefox/3.0\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";
    size = parser.ExecParser(str);
    printf("%s(%d):<%s> errno=%d size=%lld\n",
        __FILE__, __LINE__, __FUNCTION__, parser.Errno(), size);
    if (parser.Errno() != 0x7F) {
        return -3;
    }
    if (size != 0) {
        return -4;
    }
    UrlParser url1("https://www.baidu.com/s?ie=utf8&oe=utf8&wd=httplib&tn=98010089_dg&ch=3");

    int ret = url1.ExecParser();
    if (ret != 0) {
        printf("%s(%d):<%s> urlparser1 failed:%d\n",
            __FILE__, __LINE__, __FUNCTION__, ret);
        return -5;
    }
    printf("ie = %s except:utf8\n", (char*)url1["ie"]);
    printf("oe = %s except:utf8\n", (char*)url1["oe"]);
    printf("wd = %s except:httplib\n", (char*)url1["wd"]);
    printf("tn = %s except:98010089_dg\n", (char*)url1["tn"]);
    printf("ch = %s except:3\n", (char*)url1["ch"]);
    UrlParser url2("http://127.0.0.1:19811/?time=144000&salt=9527&user=test&sign=1234567890abcdef");
    ret = url2.ExecParser();
    if (ret != 0) {
        printf("%s(%d):<%s> urlparser2 failed:%d\n",
            __FILE__, __LINE__, __FUNCTION__, ret);
        return -6;
    }
    printf("time = %s except:144000\n", (char*)url2["time"]);
    printf("salt = %s except:9527\n", (char*)url2["salt"]);
    printf("user = %s except:test\n", (char*)url2["user"]);
    printf("sign = %s except:1234567890abcdef\n", (char*)url2["sign"]);
    printf("host:%s port:%d\n", (char*)url2.Host(), url2.Port());
    return 0;
}

#include "Sqlite3Client.h"
DECLARE_TABLE_CLASS(user_test, _sqlite3_table_)
DECLARE_FIELD(TYPE_INT, user_id, NOT_NULL | PRIMARY_KEY | AUTOINCREMENT, "INTEGER", "", "", "")
DECLARE_FIELD(TYPE_VARCHAR, user_qq, NOT_NULL, "VARCHAR", "(15)", "", "")
DECLARE_FIELD(TYPE_VARCHAR, user_phone, NOT_NULL|DEFAULT, "VARCHAR", "(12)", "18888888888", "")
DECLARE_FIELD(TYPE_TEXT, user_name, 0, "TEXT", "", "", "")
DECLARE_TABLE_CLASS_END()

//class user_test :public _sqlite3_table_
//{
//public:
//    user_test() :_sqlite3_table_() {
//        Name = "user_test";
//
//        PField pField = std::make_shared<_sqlite3_field_>(
//            TYPE_INT, "user_id", NOT_NULL | PRIMARY_KEY | AUTOINCREMENT, "INT", "", "", "");
//        VecField.push_back(pField);
//        MapFields["user_id"] = pField;
//
//        pField = std::make_shared< _sqlite3_field_>(
//            TYPE_VARCHAR, "user_qq", NOT_NULL | PRIMARY_KEY | AUTOINCREMENT, "VARCHAR", "(15)", "", "");
//        VecField.push_back(pField);
//        MapFields["user_qq"] = pField;           
//    }
//
//    virtual PTable Copy()const {
//        return std::make_shared<user_test>(*this);
//    }
//};

#include "MysqlClient.h"
DECLARE_TABLE_CLASS(user_test_mysql, _mysql_table_)
DECLARE_MYSQL_FIELD(TYPE_INT, user_id, NOT_NULL | PRIMARY_KEY | AUTOINCREMENT, "INTEGER", "", "", "")
DECLARE_MYSQL_FIELD(TYPE_VARCHAR, user_qq, NOT_NULL, "VARCHAR", "(15)", "", "")
DECLARE_MYSQL_FIELD(TYPE_VARCHAR, user_phone, NOT_NULL|DEFAULT, "VARCHAR", "(12)", "18888888888", "")
DECLARE_MYSQL_FIELD(TYPE_TEXT, user_name, 0, "TEXT", "", "", "")
DECLARE_TABLE_CLASS_END()


void Sqlite3_test() {
    user_test test, value;
    printf("creat:%s\n", (char*)test.TCreate());
    printf("Delete:%s\n", (char*)test.Delete(test));
    value.MapFields["user_qq"]->LoadFromStr("3186385448");
    value.MapFields["user_qq"]->Condition = SQL_INSERT;
    printf("Insert:%s\n", (char*)test.Insert(value));
    value.MapFields["user_qq"]->LoadFromStr("123456789");
    value.MapFields["user_qq"]->Condition = SQL_MODIFY;
    printf("Modify:%s\n", (char*)test.Modify(value));
    printf("Query:%s\n", (char*)test.Query());
    printf("Drop:%s\n", (char*)test.Drop());
    getchar();

    CDatabaseClient* pClient = new CSqlite3Client();
    std::map<Buffer, Buffer> args;
    args["host"] = "test.db";
    int ret=pClient->Connect(args);
    printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);
    ret=pClient->Exec(test.TCreate());
    printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);
    ret=pClient->Exec(test.Delete(value));
    printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);
    value.MapFields["user_qq"]->LoadFromStr("3186385448");
    value.MapFields["user_qq"]->Condition = SQL_INSERT;
    ret=pClient->Exec(test.Insert(value));
    printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);
    value.MapFields["user_qq"]->LoadFromStr("123456789");
    value.MapFields["user_qq"]->Condition = SQL_MODIFY;
    ret=pClient->Exec(test.Modify(value));
    printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);

    std::list<PTable> result;
    ret=pClient->Exec(test.Query(), result, test);
    printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);
    ret=pClient->Exec(test.Drop());
    printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);
    ret=pClient->Close();
    printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);
}

void mysql_test() {
    user_test_mysql test, value;
    printf("creat:%s\n", (char*)test.TCreate());
    printf("Delete:%s\n", (char*)test.Delete(test));
    value.MapFields["user_qq"]->LoadFromStr("3186385448");
    value.MapFields["user_qq"]->Condition = SQL_INSERT;
    printf("Insert:%s\n", (char*)test.Insert(value));
    value.MapFields["user_qq"]->LoadFromStr("123456789");
    value.MapFields["user_qq"]->Condition = SQL_MODIFY;
    printf("Modify:%s\n", (char*)test.Modify(value));
    printf("Query:%s\n", (char*)test.Query());
    printf("Drop:%s\n", (char*)test.Drop());
    getchar();

    CDatabaseClient* pClient = new CMysqlClient();
    std::map<Buffer, Buffer> args;
    args["host"] = "192.168.6.128";
    args["user"] = "root";
    args["password"] = "hu20010618";
    args["db"] = "HPlayer";
    args["port"] = "3306";
    int ret = pClient->Connect(args);
    printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);
    ret = pClient->Exec(test.TCreate());
    printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);
    ret = pClient->Exec(test.Delete(value));
    printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);
    value.MapFields["user_qq"]->LoadFromStr("3186385448");
    value.MapFields["user_qq"]->Condition = SQL_INSERT;
    ret = pClient->Exec(test.Insert(value));
    printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);
    value.MapFields["user_qq"]->LoadFromStr("123456789");
    value.MapFields["user_qq"]->Condition = SQL_MODIFY;
    ret = pClient->Exec(test.Modify(value));
    printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);

    std::list<PTable> result;
    ret = pClient->Exec(test.Query(), result, test);
    printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);
    ret = pClient->Exec(test.Drop());
    printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);
    ret = pClient->Close();
    printf("%s(%d):<%s> ret=%d\n", __FILE__, __LINE__, __FUNCTION__, ret);
}

int main() {
    //http_test();
    //Sqlite3_test();
    mysql_test();
    return 0;
}