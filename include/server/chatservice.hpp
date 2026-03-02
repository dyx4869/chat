#ifndef CHATSERVICE_HPP
#define CHATSERVICE_HPP

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>
#include "json.hpp"
#include "usermodel.hpp"
#include "datetime.hpp"
#include "offlinemsgmodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"
using json = nlohmann::json;
using namespace muduo;
using namespace muduo::net;
using MsgHandler = std::function<void(const TcpConnectionPtr&, json&, Timestamp)>;

class ChatService
{
private:
    std::unordered_map<int, MsgHandler> msgHandlerMap_;
    std::unordered_map<int, TcpConnectionPtr> userConnectionMap_; // SLT容器并没有考虑线程安全
    std::mutex connMutex_;
    UserModel userModel_;
    offlineMsgModel offlineMsgMod_;
    FriendModel friendModel_;
    GroupModel groupModel_;
    Redis redis_;
    ChatService();

public:
    static ChatService* instance();
    ~ChatService();
    // 根据业务代号获取业务的仿函数
    MsgHandler getHandler(int msgid);
    // 服务器异常业务重置
    void reset();
    // 客户端异常退出处理函数
    void clientCloseException(const TcpConnectionPtr& conn);
    // 登录业务
    void log(const TcpConnectionPtr& conn, json& js, Timestamp time);
    void loginout(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 注册业务
    void reg(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 私聊
    void oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time);
    void addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 建群
    void createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 加群
    void addGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 发群消息
    void groupChat(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 服务器间通信
    void serverCommunication(int channel, std::string message);
};


#endif