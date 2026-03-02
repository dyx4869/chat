#pragma once
#include "user.hpp"
class UserModel
{
private:
public:
    // 用户表的增加方法
    bool insert(User& user);
    // 根据用户ID获取用户信息
    User query(int id);
    // 更新用户的状态信息
    bool updateState(User user);

    bool resetState();
};
