#ifndef PUBLIC_H
#define PUBLIC_H

enum EnMsgType {
    LOGIN_MSG = 1, // 登录消息
    LOGINOUT_MSG,
    LOGIN_MSG_ACK,    // 登录响应消息
    REG_MSG,          // 注册消息
    REG_MSG_ACK,      // 注册响应消息
    ONE_CHAT_MSG,     // 私聊消息 msgid id:1 from:"zhang san" to:3 msg:"hello"
    ADD_FRIEND_MSG,   // 添加好友
    GROUP_CHAT_MSG,   // 群聊
    CREATE_GROUP,     // 建群
    ADD_GROUP,        // 加群
    CREATE_GROUP_ACK, // 建群响应消息
    ADD_GROUP_ACK,    // 加群响应消息

};

#endif