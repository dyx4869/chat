#include "offlinemsgmodel.hpp"
#include "db.h"

bool offlineMsgModel::insert(int toid, std::string msg)
{
    char sql[1024];
    sprintf(sql, "insert into  OfflineMessage values(%d,'%s')", toid, msg.c_str());
    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            return true;
        }
    }
    return false;
}
bool offlineMsgModel::remove(int toid)
{
    char sql[1024];
    sprintf(sql, "delete from  OfflineMessage where toid = %d ", toid);

    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            return true;
        }
    }
    return false;
}
std::vector<std::string> offlineMsgModel::query(int toid)
{
    char sql[1024];
    sprintf(sql, "select message from  OfflineMessage where toid = %d", toid);
    std::vector<std::string> vec;
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql); // 查询后返回一个动态开辟的内存结果指针
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                json js;
                js["msg"] = row[0];
                vec.push_back(js.dump());
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}