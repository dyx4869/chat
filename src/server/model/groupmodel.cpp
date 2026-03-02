#include "groupmodel.hpp"
#include "db.h"

bool GroupModel::createGroup(Group& group)
{
    char sql[1024];
    sprintf(sql, "insert into AllGroup(groupname,groupdesc) values('%s','%s')", group.getName().c_str(),
        group.getDesc().c_str());

    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

bool GroupModel::isGroupExist(int groupid)
{
    char sql[1024];
    sprintf(sql, "select * from AllGroup where id = %d", groupid);
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql); // 查询后返回一个动态开辟的内存结果指针
        if (res != nullptr) {
            mysql_free_result(res);
            return true;
        }
    }
    return false;
}

bool GroupModel::addGroup(int userid, int groupid, std::string role)
{
    char sql[1024];
    sprintf(sql, "insert into GroupUser(groupid,userid,grouprole) values(%d,%d,'%s')", groupid, userid,
        role.c_str());

    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            return true;
        }
    }
    return false;
}

std::vector<Group> GroupModel::queryGroup(int userid)
{
    char sql[1024];
    sprintf(sql,
        "select a.id,a.groupname,a.groupdesc from AllGroup a inner join  GroupUser b on a.id = b.groupid "
        "where b.userid = %d",
        userid);
    std::vector<Group> vec;
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql); // 查询后返回一个动态开辟的内存结果指针
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                vec.push_back(group);
            }
            mysql_free_result(res);
        }
    }

    for (Group& group : vec) {
        sprintf(sql,
            "select a.id,a.name,a.state,b.grouprole from User a inner join GroupUser b on b.userid = a.id "
            "where b.groupid = %d",
            group.getId());


        MYSQL_RES* res = mysql.query(sql); // 查询后返回一个动态开辟的内存结果指针
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                GroupUser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                group.getUsers().push_back(user);
            }
            mysql_free_result(res);
        }
    }

    return vec;
}

std::vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{
    char sql[1024];
    sprintf(sql, "select userid from GroupUser where groupid = %d and userid != %d", groupid, userid);
    std::vector<int> vec;
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql); // 查询后返回一个动态开辟的内存结果指针
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                vec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}