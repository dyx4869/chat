#ifndef DB_H
#define DB_H
#include <string>
#include <mysql/mysql.h>


class MySQL
{
private:
    MYSQL* conn_;

public:
    MySQL();
    ~MySQL();
    bool connect();
    bool update(std::string sql);
    MYSQL_RES* query(std::string sql);
    MYSQL* getConnection();
};


#endif