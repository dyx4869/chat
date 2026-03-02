#ifndef OFFLINEUSERMSGMODEL_H
#define OFFLINEUSERMSGMODEL_H
#include <string>
#include "json.hpp"
#include <vector>
using json = nlohmann::json;
class offlineMsgModel
{
public:
    bool insert(int toid, std::string msg);
    bool remove(int toid);
    std::vector<std::string> query(int toid);
};


#endif