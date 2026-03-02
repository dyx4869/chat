#include "db.h"
#include <muduo/base/Logging.h>


static std::string server = "127.0.0.1";
static std::string user = "root";
static std::string password = "123456";
static std::string dbname = "chat";


MySQL::MySQL() { conn_ = mysql_init(nullptr); }
MySQL::~MySQL()
{
    if (conn_ != nullptr) {
        mysql_close(conn_);
    }
}

bool MySQL::connect()
{
    MYSQL* p = mysql_real_connect(
        conn_, server.c_str(), user.c_str(), password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if (p != nullptr) {
        // c/c++ 的默认编码是ASCII，不设置，从数据库得到的数据会乱码
        LOG_INFO << "connect sql success";
        mysql_query(conn_, "set names gbk");
    } else
        LOG_INFO << "connect sql failed";
    return p;
}
bool MySQL::update(std::string sql)
{
    if (mysql_query(conn_, sql.c_str())) {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "update failed";
        return false;
    }
    return true;
}
MYSQL_RES* MySQL::query(std::string sql)
{
    if (mysql_query(conn_, sql.c_str())) {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "query failed";
        return nullptr;
    }
    return mysql_use_result(conn_);
}

MYSQL* MySQL::getConnection() { return conn_; }