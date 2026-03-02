#include "usermodel.hpp"
#include "db.h"
#include <iostream>

bool UserModel::insert(User& user)
{
    char sql[1024];
    sprintf(sql, "insert into User(name,passwd,state) values('%s','%s','%s')", user.getName().c_str(),
        user.getPasswd().c_str(), user.getState().c_str());

    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            user.setId(mysql_insert_id(mysql.getConnection()));

            return true;
        }
    }
    return false;
}

User UserModel::query(int id)
{
    char sql[1024];
    sprintf(sql, "select * from User where id = %d", id);
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql); // 查询后返回一个动态开辟的内存结果指针
        if (res != nullptr) {
            MYSQL_ROW row = mysql_fetch_row(res); // 返回一行查询结果
            if (row != nullptr) {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPasswd(row[2]);
                user.setState(row[3]);
                mysql_free_result(res); // 释放内存
                return user;
            }
        }
    }
    return User();
}

bool UserModel::updateState(User user)
{
    char sql[1024];
    sprintf(sql, "update User set state = '%s' where id = %d", user.getState().c_str(), user.getId());

    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            return true;
        }
    }
    return false;
}

bool UserModel::resetState()
{
    char sql[1024] = "update User set state = 'offline' where state = 'online'";

    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            return true;
        }
    }
    return false;
}