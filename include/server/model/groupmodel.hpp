#pragma once
#include "groupuser.hpp"
#include "group.hpp"
class GroupModel
{
public:
    // userid注册一个群聊
    bool createGroup(Group& group);
    // 加入一个群
    bool addGroup(int userid, int groupid, std::string role);
    bool isGroupExist(int groupid);
    // 查看用户所在的所有群组信息，登录时推送
    std::vector<Group> queryGroup(int userid);
    // 根据指定的userid，groupid查询这个group中除该用户的所有id用于服务器转发该用户在群内发送的消息
    std::vector<int> queryGroupUsers(int userid, int groupid);
};
