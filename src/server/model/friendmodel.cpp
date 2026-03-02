#include "friendmodel.hpp"
#include "db.h"
bool FriendModel::insert(int userid, int friendid)
{
    char sql[1024];
    sprintf(sql, "insert into Friend values(%d,%d)", userid, friendid);

    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            return true;
        }
    }
    return false;
}

std::vector<User> FriendModel::query(int userid)
{
    char sql[1024];
    sprintf(sql,
        "select a.id,a.name,a.state from User a inner join Friend b on b.friendid = a.id where b.userid = %d",
        userid);
    std::vector<User> vec;
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql); // 查询后返回一个动态开辟的内存结果指针
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}