#pragma once
#include "user.hpp"
class GroupUser : public User
{
private:
    std::string role;

public:
    void setRole(std::string role) { this->role = role; }
    std::string getRole() { return role; }
};
