#pragma once
#include <hiredis/hiredis.h>
#include <functional>
#include <thread>
#include <string>

class Redis
{
private:
    redisContext* publish_context_;

    redisContext* subscribe_context_;

    std::function<void(int, std::string)> notify_message_handler_;

public:
    Redis();
    ~Redis();
    // 连接redis服务器
    bool connect();
    // 向指定通道发布消息
    bool publish(int channel, std::string message);
    // 向指定通道订阅消息
    bool subscribe(int channel);
    // 像指定通道取消订阅
    bool unsubscribe(int channel);
    // 在独立线程中接收订阅通道中的消息
    void observer_channel_message();
    void init_notify_handler(std::function<void(int, std::string)> fn);
};
