#ifndef CHATSERVER_H
#define CHATSERVER_H
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
// #include<muduo/net/Buffer.h>
using namespace muduo;
using namespace muduo::net;
class ChatServer
{
private:
    EventLoop* loop_;
    TcpServer server_;
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time);
    void onConnection(const TcpConnectionPtr& conn);

public:
    ChatServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& name);

    void start();
};

#endif