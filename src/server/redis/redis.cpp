#include "redis.hpp"
#include <iostream>
Redis::Redis()
    : publish_context_(nullptr)
    , subscribe_context_(nullptr)
{
}
Redis::~Redis()
{
    if (publish_context_ != nullptr)
        redisFree(publish_context_);
    if (subscribe_context_ != nullptr)
        redisFree(subscribe_context_);
}
// 连接redis服务器
bool Redis::connect()
{
    publish_context_ = redisConnect("127.0.0.1", 6379);
    if (nullptr == publish_context_) {
        std::cerr << "redis connect failed" << std::endl;
        return false;
    }
    subscribe_context_ = redisConnect("127.0.0.1", 6379);
    if (nullptr == subscribe_context_) {
        std::cerr << "redis connect failed" << std::endl;
        return false;
    }
    redisReply* reply = (redisReply*)redisCommand(publish_context_, "AUTH 123456");
    if (nullptr == reply) {
        std::cerr << "publish command failed" << std::endl;
        return false;
    }
    freeReplyObject(reply);

    reply = (redisReply*)redisCommand(subscribe_context_, "AUTH 123456");
    if (nullptr == reply) {
        std::cerr << "publish command failed" << std::endl;
        return false;
    }
    freeReplyObject(reply);

    std::thread t([&]() { observer_channel_message(); });
    t.detach();
    return true;
}
// 向指定通道发布消息
bool Redis::publish(int channel, std::string message)
{
    redisReply* reply =
        (redisReply*)redisCommand(publish_context_, "PUBLISH %d %s", channel, message.c_str());
    if (nullptr == reply) {
        std::cerr << "publish command failed" << std::endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}
// 向指定通道订阅消息
bool Redis::subscribe(int channel)
{
    // subscribe命令会阻塞线程等待通道消息，这里只订阅，不接收消息
    // 在observer_channel_message线程中接收消息
    if (REDIS_ERR == redisAppendCommand(this->subscribe_context_, "SUBSCRIBE %d", channel)) {
        std::cerr << "subscribe command failed" << std::endl;
        return false;
    }
    int done = 0;
    // redisBufferWrite可以循环发送缓冲区，直到缓冲区的数据发完(done被置为1)
    while (!done) {
        if (REDIS_ERR == redisBufferWrite(this->subscribe_context_, &done)) {
            std::cerr << "subscribe command failed" << std::endl;
            return false;
        }
    }
    return true;
}
// 像指定通道取消订阅
bool Redis::unsubscribe(int channel)
{
    // subscribe命令会阻塞线程等待通道消息，这里只订阅，不接收消息
    // 在observer_channel_message线程中接收消息
    if (REDIS_ERR == redisAppendCommand(this->subscribe_context_, "UNSUBSCRIBE %d", channel)) {
        std::cerr << "unsubscribe command failed" << std::endl;
        return false;
    }
    int done = 0;
    // redisBufferWrite可以循环发送缓冲区，直到缓冲区的数据发完(done被置为1)
    while (!done) {
        if (REDIS_ERR == redisBufferWrite(this->subscribe_context_, &done)) {
            std::cerr << "unsubscribe command failed" << std::endl;
            return false;
        }
    }
    return true;
}
// 在独立线程中接收订阅通道中的消息
void Redis::observer_channel_message()
{
    redisReply* reply = nullptr;
    while (REDIS_OK == redisGetReply(this->subscribe_context_, (void**)&reply)) {
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr) {
            notify_message_handler_(atoi(reply->element[1]->str), reply->element[2]->str);
        }
        freeReplyObject(reply);
    }
    std::cerr << "======================observer_channel_message quit======================" << std::endl;
}
void Redis::init_notify_handler(std::function<void(int, std::string)> fn) { notify_message_handler_ = fn; }