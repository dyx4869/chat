#include "chatserver.hpp"
#include "chatservice.hpp"
#include "json.hpp"
#include <functional>
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& name)
    : loop_(loop)
    , server_(loop, listenAddr, name)
{
    server_.setConnectionCallback(std::bind(&ChatServer ::onConnection, this, std::placeholders::_1));
    server_.setMessageCallback(std::bind(
        &ChatServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    server_.setThreadNum(4);
}

void ChatServer::start() { server_.start(); }

void ChatServer::onConnection(const TcpConnectionPtr& conn)
{
    if (!conn->connected()) {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}
void ChatServer::onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time)
{
    std::string buf = buffer->retrieveAllAsString();
    json js = json::parse(buf); // 数据的反序列化
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    msgHandler(conn, js, time);
}