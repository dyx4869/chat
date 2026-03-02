#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <string>
#include <vector>
#include "usermodel.hpp"

/*
    errno
    1   注册用户失败
    2   登录失败，已经登录
    3   登录失败，登录密码错误
    4   登录失败，登录的用户不存在
    5   创建群聊失败
    6   设置群聊创建者失败
    7   加群失败
    8   群聊不存在

*/

ChatService* ChatService::instance()
{
    static ChatService service;
    return &service;
}
ChatService::ChatService()
{
    msgHandlerMap_.insert({LOGIN_MSG,
        std::bind(
            &ChatService::log, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    msgHandlerMap_.insert({REG_MSG,
        std::bind(
            &ChatService::reg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});

    msgHandlerMap_.insert({ONE_CHAT_MSG,
        std::bind(&ChatService::oneChat, this, std::placeholders::_1, std::placeholders::_2,
            std::placeholders::_3)});

    msgHandlerMap_.insert({ADD_FRIEND_MSG,
        std::bind(&ChatService::addFriend, this, std::placeholders::_1, std::placeholders::_2,
            std::placeholders::_3)});
    msgHandlerMap_.insert({CREATE_GROUP,
        std::bind(&ChatService::createGroup, this, std::placeholders::_1, std::placeholders::_2,
            std::placeholders::_3)});
    msgHandlerMap_.insert({ADD_GROUP,
        std::bind(&ChatService::addGroup, this, std::placeholders::_1, std::placeholders::_2,
            std::placeholders::_3)});

    msgHandlerMap_.insert({GROUP_CHAT_MSG,
        std::bind(&ChatService::groupChat, this, std::placeholders::_1, std::placeholders::_2,
            std::placeholders::_3)});
    msgHandlerMap_.insert({LOGINOUT_MSG,
        std::bind(&ChatService::loginout, this, std::placeholders::_1, std::placeholders::_2,
            std::placeholders::_3)});
    if (redis_.connect()) {
        redis_.init_notify_handler(
            std::bind(&ChatService::serverCommunication, this, std::placeholders::_1, std::placeholders::_2));
    }
}


ChatService::~ChatService() { }

void ChatService::reset()
{
    // 将online用户设置为offline
    userModel_.resetState();
}

MsgHandler ChatService::getHandler(int msgid)
{
    auto it = msgHandlerMap_.find(msgid);
    if (it == msgHandlerMap_.end()) {
        return [=](const TcpConnectionPtr& conn, json& js, Timestamp time) {
            LOG_ERROR << "msgid:" << msgid << "can not find handler";
        };
    } else
        return msgHandlerMap_[msgid];
}

void ChatService::clientCloseException(const TcpConnectionPtr& conn)
{
    User user;
    {
        std::lock_guard<std::mutex> lock(connMutex_);
        for (auto it = userConnectionMap_.begin(); it != userConnectionMap_.end(); ++it) {
            if (it->second == conn) {
                user.setId(it->first);
                userConnectionMap_.erase(it);
                break;
            }
        }
    }
    redis_.unsubscribe(user.getId());

    if (user.getId() != -1) {
        user.setState("offline");
        userModel_.updateState(user);
    }
}

void ChatService::log(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int id = js["id"].get<int>();
    std::string passwd = js["passwd"];
    User user = userModel_.query(id);
    if (user.getId() == -1) {
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errmsg"] = "登录的用户不存在";
        response["errno"] = 4; // errno=4  登录失败，登录的用户不存在
        conn->send(response.dump());
    } else {
        if (user.getPasswd() == passwd && user.getId() != -1) {
            // successful
            if (user.getState() == "online") {
                json response;
                response["msgid"] = LOGIN_MSG_ACK;
                response["errno"] = 2; // errno=2  登录失败，已经登录
                response["errmsg"] = "该账号已经登录";
                conn->send(response.dump());

            } else {
                // 登录成功，记录用户连接信息
                {
                    // std::unique_lock<std::mutex> lock(connMutex_);
                    std::lock_guard<std::mutex> lock(connMutex_);
                    userConnectionMap_.insert({id, conn});
                }
                // 更改登陆状态
                user.setState("online");
                userModel_.updateState(user);

                redis_.subscribe(id);

                json response;
                response["msgid"] = LOGIN_MSG_ACK;
                response["errno"] = 0;
                response["id"] = user.getId();
                response["name"] = user.getName();
                // 接收离线消息
                std::vector<std::string> vec = offlineMsgMod_.query(id);
                if (!vec.empty()) {
                    response["offline"] = vec;
                    offlineMsgMod_.remove(id);
                }
                // 查询好友信息
                std::vector<User> uservec = friendModel_.query(id);
                if (!uservec.empty()) {
                    std::vector<std::string> vec2;
                    for (User& user : uservec) {
                        json userjs;
                        userjs["id"] = user.getId();
                        userjs["name"] = user.getName();
                        userjs["state"] = user.getState();
                        vec2.push_back(userjs.dump());
                    }
                    response["friends"] = vec2;
                }
                // 查询当前用户加入的群租信息
                std::vector<Group> groupvec = groupModel_.queryGroup(id);
                if (!groupvec.empty()) {
                    std::vector<std::string> vec2;
                    for (Group& group : groupvec) {
                        json gjs;
                        gjs["id"] = group.getId();
                        gjs["name"] = group.getName();
                        gjs["desc"] = group.getDesc();
                        std::vector<std::string> vec3;
                        for (GroupUser& groupuser : group.getUsers()) {
                            json test;
                            test["id"] = groupuser.getId();
                            test["name"] = groupuser.getName();
                            test["state"] = groupuser.getState();
                            test["role"] = groupuser.getRole();
                            vec3.push_back(test.dump());
                        }
                        gjs["groupusers"] = vec3;

                        vec2.push_back(gjs.dump());
                    }
                    response["groups"] = vec2;
                }

                conn->send(response.dump());
            }
        } else {
            // failed
            if (user.getPasswd() != passwd) {
                json response;
                response["msgid"] = LOGIN_MSG_ACK;
                response["errmsg"] = "登录密码错误";
                response["errno"] = 3; // errno=3  登录失败，登录密码错误
                conn->send(response.dump());
            }
        }
    }
}

void ChatService::loginout(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int id = js["id"].get<int>();
    {
        std::lock_guard<std::mutex> lock(connMutex_);
        auto it = userConnectionMap_.find(id);
        if (it != userConnectionMap_.end())
            userConnectionMap_.erase(it);
    }
    redis_.unsubscribe(id);
    User user(id);
    user.setState("offline");
    userModel_.updateState(user);
}
void ChatService::reg(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    std::string name = js["name"];
    std::string passwd = js["passwd"];
    User user;
    user.setName(name);
    user.setPasswd(passwd);
    bool state = userModel_.insert(user);
    if (state) {
        // successful
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();

        conn->send(response.dump());
    } else {
        // failed
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1; // errno=1  注册用户失败

        conn->send(response.dump());
    }
}

void ChatService::oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int toid = js["toid"].get<int>();

    {
        std::lock_guard<std::mutex> lock(connMutex_);
        auto it = userConnectionMap_.find(toid);
        // 出作用域可能对方会下线，连接会被删除
        if (it != userConnectionMap_.end()) {
            // online
            it->second->send(js.dump());
            return;
        }
    }

    User user = userModel_.query(toid);
    if ("online" == user.getState()) {
        redis_.publish(toid, js.dump());
        return;
    }

    // offline
    offlineMsgMod_.insert(toid, js.dump());
}

void ChatService::addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int userid = js["userid"].get<int>();
    int friendid = js["friendid"].get<int>();
    friendModel_.insert(userid, friendid);
    friendModel_.insert(friendid, userid);
}
/*
    5   创建群聊失败
    6   设置群聊创建者失败
    7   群聊存在，加群失败
    8   群聊不存在
*/

// 建群
void ChatService::createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int id = js["id"].get<int>();
    std::string name = js["groupname"];
    std::string desc = js["groupdesc"];
    Group group;
    group.setName(name);
    group.setDesc(desc);
    bool state = groupModel_.createGroup(group);
    json response;
    if (state) {
        if (groupModel_.addGroup(id, group.getId(), "creator")) {
            response["errno"] = 0;
            response["groupid"] = group.getId();

        } else {
            response["errno"] = 6;
        }

    } else {
        response["errno"] = 5;
    }
    response["msgid"] = CREATE_GROUP_ACK;
    conn->send(js.dump());
}
// 加群
void ChatService::addGroup(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int id = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    json response;
    if (groupModel_.isGroupExist(groupid)) {
        if (groupModel_.addGroup(id, groupid, "normal")) {
            response["errno"] = 0;
        } else {
            response["errno"] = 7;
        }
    } else {
        response["errno"] = 8;
    }
    response["msgid"] = ADD_GROUP_ACK;
    conn->send(js.dump());
}
// 发群消息
void ChatService::groupChat(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int id = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();


    std::vector<int> vec = groupModel_.queryGroupUsers(id, groupid);

    std::lock_guard<std::mutex> lock(connMutex_);
    for (int val : vec) {
        auto it = userConnectionMap_.find(val);
        if (it != userConnectionMap_.end()) {
            it->second->send(js.dump());
        } else {
            User user = userModel_.query(val);
            if ("online" == user.getState()) {
                redis_.publish(val, js.dump());

            } else
                offlineMsgMod_.insert(val, js.dump());
        }
    }
}

void ChatService::serverCommunication(int channel, std::string msg)
{
    {
        std::lock_guard<std::mutex> lock(connMutex_);
        auto it = userConnectionMap_.find(channel);
        if (it != userConnectionMap_.end()) {
            it->second->send(msg);
            return;
        }
    }


    offlineMsgMod_.insert(channel, msg);
}