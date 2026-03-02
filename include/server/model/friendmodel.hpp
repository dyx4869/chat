#pragma once
#include <vector>
#include "user.hpp"

class FriendModel
{
public:
    // 添加好友
    bool insert(int userid, int friendid);
    // 返回用户的好友列表
    std::vector<User> query(int userid);
};
