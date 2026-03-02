#pragma once
#include <string>


class User
{
private:
    int id;
    std::string name;
    std::string passwd;
    std::string state;

public:
    User(int id = -1, std::string name = "", std::string passwd = "", std::string state = "offline")
    {
        this->id = id;
        this->name = name;
        this->passwd = passwd;
        this->state = state;
    }

    void setId(int id) { this->id = id; }
    void setName(std::string name) { this->name = name; }
    void setPasswd(std::string passwd) { this->passwd = passwd; }
    void setState(std::string state) { this->state = state; }

    int getId() { return id; }
    std::string getName() { return name; }
    std::string getPasswd() { return passwd; }
    std::string getState() { return state; }
};
