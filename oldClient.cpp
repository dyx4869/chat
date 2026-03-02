#include "json.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <ctime>
#include <unordered_map>
#include <functional>


#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include "group.hpp"
#include "user.hpp"
#include "public.hpp"
// #include "datetime.hpp"

using json = nlohmann::json;
using namespace std;

User g_currentUser;
vector<User> g_currentUserFriendList;
vector<Group> g_currentUserGroupList;
// 登录退出返回聊天页面的控制变量
bool loginState = false;

void help(int fd = -1, string com = "");
void chat(int cliendfd, string com);
void addfriend(int cliendfd, string com);
void creategroup(int cliendfd, string com);
void joingroup(int cliendfd, string com);
void groupchat(int cliendfd, string com);
void loginout(int fd = -1, string com = "");


unordered_map<string, string> commandMap = {{"help", "显示所有命令,格式help"},
    {"chat", "私聊,格式chat:friendid:message"}, {"addfriend", "加好友,格式addfriend:friendid"},
    {"creategroup", "建群,格式creategroup:groupname:groupdesc"}, {"joingroup", "加群,格式joingroup:groupid"},
    {"groupchat", "群聊,格式groupchat:groupid:message"}, {"loginout", "退出登录,格式loginout"}};

unordered_map<string, function<void(int, string)>> commandHandlerMap{{"help", help}, {"chat", chat},
    {"addfriend", addfriend}, {"creategroup", creategroup}, {"joingroup", joingroup},
    {"groupchat", groupchat}, {"loginout", loginout}};


// 显示当前用户的基本信息
void showCurrentUserData();

// 接收线程回调

void readTaskHandler(int clientfd);

string getCurrentTime();

void mainMenu(int clientfd);


int main_old(int argc, char** argv)
{
    if (argc < 3) {
        cerr << "ip/port is not right example : ./chatclient 127.0.0.1 10000" << endl;
        exit(-1);
    }
    char* ip = argv[1];
    uint16_t port = atoi(argv[2]);


    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd) {
        cout << "socket create failed" << endl;
        exit(-1);
    }

    sockaddr_in saddr;
    memset(&saddr, 0, sizeof(sockaddr_in));
    socklen_t saddrlen = sizeof(saddr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = inet_addr(ip);

    int res = connect(clientfd, (sockaddr*)&saddr, saddrlen);
    if (res == -1) {
        cerr << "connect server failed" << endl;
        close(clientfd);
        exit(-1);
    }

    for (;;) {
        cout << "============================================================" << endl;
        cout << "1.login" << endl;
        cout << "2.register" << endl;
        cout << "3.quit" << endl;
        cout << "============================================================" << endl;
        cout << "choice:" << endl;
        int choice = 0;
        cin >> choice;
        cin.get(); // 读掉缓冲区残留的回车

        switch (choice) {
        case 1: {
            int id = 0;
            char userpasswd[50];
            cout << "please input userid:" << endl;
            cin >> id;
            cin.get();
            cout << "please input userpasswd:" << endl;
            cin.getline(userpasswd, 50);

            json js;
            js["msgid"] = LOGIN_MSG;
            js["id"] = id;
            js["passwd"] = userpasswd;
            string strjs = js.dump();
            int len = send(clientfd, strjs.c_str(), strlen(strjs.c_str()) + 1, 0);
            if (-1 == len) {
                cerr << "send login message failed" << endl;
            } else {
                char buf[1024] = {0};
                len = recv(clientfd, buf, 1024, 0);
                if (len == -1) {
                    cerr << "recv login response failed" << endl;
                } else {
                    json response = json::parse(buf);

                    int errnum = response["errno"].get<int>();
                    if (0 != errnum) {
                        switch (errnum) {
                        case 2:
                            cerr << "登录失败，已经登录" << endl;
                            break;

                        case 3:
                            cerr << "登录失败，登录密码错误" << endl;
                            break;
                        case 4:
                            cerr << "登录失败，登录的用户不存在" << endl;
                            break;
                        }
                    } else {
                        // 记录用户的ID和name
                        g_currentUser.setId(response["id"].get<int>());
                        g_currentUser.setName(response["name"]);
                        // 记录当前用户的好友列表
                        if (response.contains("friends")) {
                            vector<string> vec = response["friends"];
                            for (string& str : vec) {
                                json temp = json::parse(str);
                                User user;
                                user.setId(temp["id"].get<int>());
                                user.setName(temp["name"]);
                                user.setState(temp["state"]);
                                g_currentUserFriendList.push_back(user);
                            }
                        }
                        // 记录当前用户的群组信息
                        if (response.contains("groups")) {
                            vector<string> v1 = response["groups"];
                            for (string& str1 : v1) {
                                json j1 = json::parse(str1);
                                Group group;
                                group.setId(j1["id"].get<int>());
                                group.setName(j1["name"]);
                                group.setDesc(j1["desc"]);
                                vector<string> v2 = j1["groupusers"];
                                for (string& str2 : v2) {
                                    json j2 = json::parse(str2);
                                    GroupUser user;
                                    user.setId(j2["id"].get<int>());
                                    user.setName(j2["name"]);
                                    user.setState(j2["state"]);
                                    user.setRole(j2["role"]);
                                    group.getUsers().push_back(user);
                                }
                                g_currentUserGroupList.push_back(group);
                            }
                        }

                        //
                        showCurrentUserData();

                        if (response.contains("offline")) {
                            vector<string> v = response["offline"];
                            for (string& str1 : v) {
                                json j2 = json::parse(str1);
                                string str2 = j2["msg"];
                                json j1 = json::parse(str2);
                                if (j1["msgid"].get<int>() == ONE_CHAT_MSG) {
                                    cout << "time: " << j1["time"] << "\t" << "sendid: " << j1["id"] << endl;
                                    cout << j1["from"] << " said: " << j1["msg"] << endl;
                                }
                                if (j1["msgid"].get<int>() == GROUP_CHAT_MSG) {
                                    cout << "time: " << j1["time"] << "\t" << "in group:" << j1["groupid"]
                                         << "\t" << "sendid: " << j1["id"] << endl;
                                    cout << j1["from"] << " said: " << j1["msg"] << endl;
                                }
                            }
                        }
                        static int readThreadNum = 0;
                        if (readThreadNum == 0) {
                            std::thread readTask(readTaskHandler, clientfd);
                            readTask.detach();
                            readThreadNum++;
                        }

                        loginState = true;
                        mainMenu(clientfd);
                    }
                }
            }


        } break;
        case 2: {
            char username[50];
            char userpasswd[50];
            cout << "please input username" << endl;
            cin.getline(username, 50);
            cout << "please input userpasswd" << endl;
            cin.getline(userpasswd, 50);

            json js;
            js["name"] = username;
            js["passwd"] = userpasswd;
            js["msgid"] = REG_MSG;
            string strjs = js.dump();

            int len = send(clientfd, strjs.c_str(), strlen(strjs.c_str()) + 1, 0);
            if (len == -1) {
                cerr << "register send failed" << endl;
            } else {
                char buf[1024] = {0};
                len = recv(clientfd, buf, 1024, 0);
                if (-1 == len) {
                    cerr << "recv register response failed" << endl;
                } else {
                    json response = json::parse(buf);

                    if (response["errno"].get<int>() == 0) {
                        cout << "id : " << response["id"].get<int>() << " " << "passwd : " << userpasswd
                             << endl;
                        cout << "do not forget id passwd" << endl;
                    } else {
                        cerr << "register failed" << endl;
                    }
                }
            }


        } break;
        case 3:
            exit(0);
            break;


        default:
            cerr << "invalid input" << endl;
            break;
        }
    }


    return 0;
}


void showCurrentUserData()
{
    cout << "=========================login user=========================" << endl;
    cout << "current login user id : " << g_currentUser.getId() << "name : " << g_currentUser.getName()
         << endl;
    cout << "-------------------------friend list------------------------" << endl;
    if (!g_currentUserFriendList.empty()) {
        for (auto& user : g_currentUserFriendList) {
            cout << "userid: " << user.getId() << " " << user.getName() << " " << user.getState() << endl;
        }
    }
    cout << "-------------------------group list-------------------------" << endl;
    if (!g_currentUserGroupList.empty()) {
        for (Group& group : g_currentUserGroupList) {
            cout << "groupid: " << group.getId() << " " << group.getName() << " " << group.getDesc() << endl;
            for (GroupUser& user : group.getUsers()) {
                cout << "\tuserid: " << user.getId() << " " << user.getName() << " " << user.getState() << " "
                     << user.getRole() << endl;
            }
        }
    }
    cout << "============================================================" << endl;
}

void readTaskHandler(int clientfd)
{
    for (;;) {
        char buf[1024] = {0};
        int len = recv(clientfd, buf, 1024, 0);
        if (-1 == len || 0 == len) {
            close(clientfd);
            exit(-1);
        }
        json js = json::parse(buf);
        int msgtype = js["msgid"].get<int>();
        if (ONE_CHAT_MSG == msgtype) {
            cout << "time: " << js["time"] << "\t" << "id: " << js["id"] << endl;
            cout << js["from"] << " said: " << js["msg"] << endl;
            continue;
        }
        if (GROUP_CHAT_MSG == msgtype) {
            cout << "time: " << js["time"] << "\t" << "in group:" << js["groupid"] << "\t"
                 << "sendid: " << js["id"] << endl;
            cout << js["from"] << " said: " << js["msg"] << endl;
            continue;
        }


        if (CREATE_GROUP_ACK == msgtype) {
            if (js["errno"].get<int>() == 0) {
                cout << "creategroup is successful" << endl;
            }
            if (js["errno"].get<int>() == 5) {
                cout << "创建群聊失败" << endl;
            }
            if (js["errno"].get<int>() == 6) {
                cout << "设置群聊创建者失败" << endl;
            }

            continue;
        }
        if (ADD_GROUP_ACK == msgtype) {
            if (js["errno"].get<int>() == 0) {
                cout << "joingroup is successful" << endl;
            }
            if (js["errno"].get<int>() == 7) {
                cout << "群聊存在，但加群失败" << endl;
            }
            if (js["errno"].get<int>() == 8) {
                cout << "群聊不存在，加群失败" << endl;
            }
            continue;
        }
    }
}

string getCurrentTime()
{
    char buf[1024];
    auto temp = time(NULL);
    tm* tm_time = localtime(&temp);
    snprintf(buf, 128, "%4d-%02d-%02d %02d:%02d:%02d", tm_time->tm_year + 1900, tm_time->tm_mon + 1,
        tm_time->tm_mday, tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);
    return buf;
}


void mainMenu(int clientfd)
{
    help();
    char buf[1024] = {0};
    while (loginState) {
        cin.getline(buf, 1024);
        string commandbuf(buf);
        string command;
        int idx = commandbuf.find(":");
        if (-1 == idx) {
            command = commandbuf;
        } else {
            command = commandbuf.substr(0, idx);
            // 从0下标，一共idx个字符
        }
        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end()) {
            cerr << "this command is invalid" << endl;
            continue;
        }

        it->second(clientfd, commandbuf.substr(idx + 1, commandbuf.size() - idx));
        continue;
    }
}
void help(int fd, string)
{
    cout << "============================================================" << endl;
    cout << "all command" << endl;
    for (auto& it : commandMap) {
        if (it.first == "help" || it.first == "chat") {
            cout << it.first << "\t\t\t" << it.second << endl;
        } else
            cout << it.first << "\t\t" << it.second << endl;
    }
    cout << "============================================================" << endl;
}

void chat(int cliendfd, string com)
{
    int idx = com.find(":");
    if (idx == -1) {
        cerr << "onchat command invalid" << endl;
        return;
    }
    int friendid = atoi(com.substr(0, idx).c_str());
    string msg = com.substr(idx + 1, com.size() - idx);
    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["from"] = g_currentUser.getName();
    js["toid"] = friendid;
    js["msg"] = msg;
    js["time"] = getCurrentTime();
    string str = js.dump();
    int len = send(cliendfd, str.c_str(), strlen(str.c_str()) + 1, 0);
    if (len == -1) {
        cerr << "onechat send failed : " << str << endl;
    }
}
void addfriend(int cliendfd, string com)
{
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["userid"] = g_currentUser.getId();
    js["friendid"] = atoi(com.c_str());

    string str = js.dump();
    int len = send(cliendfd, str.c_str(), strlen(str.c_str()) + 1, 0);
    if (len == -1) {
        cerr << "addfriend send failed : " << str << endl;
    }
} /*
 unordered_map<string, string> commandMap = {{"help", "显示所有命令,格式help"},
     {"chat", "私聊,格式chat:friendid:message"}, {"addfriend", "加好友,格式addfriend:friendid"},
     {"creategroup", "建群,格式creategroup:groupname:groupdesc"}, {"joingroup", "加群,格式joingroup:groupid"},
     {"groupchat", "群聊,格式:groupchat:groupid:message"}, {"loginout", "退出登录,格式loginout"}};
 */
void creategroup(int cliendfd, string com)
{
    int idx = com.find(":");
    if (idx == -1) {
        cerr << "command invalid" << endl;
        return;
    }
    string name = com.substr(0, idx);
    string desc = com.substr(idx + 1, com.size() - idx);
    json js;
    js["id"] = g_currentUser.getId();
    js["groupname"] = name;
    js["groupdesc"] = desc;
    js["msgid"] = CREATE_GROUP;

    string str = js.dump();
    int len = send(cliendfd, str.c_str(), strlen(str.c_str()) + 1, 0);
    if (len == -1) {
        cerr << "creategroup send failed : " << str << endl;
    }
}
void joingroup(int cliendfd, string com)
{
    json js;
    js["msgid"] = ADD_GROUP;
    js["id"] = g_currentUser.getId();
    js["groupid"] = atoi(com.c_str());

    string str = js.dump();
    int len = send(cliendfd, str.c_str(), strlen(str.c_str()) + 1, 0);
    if (len == -1) {
        cerr << "addfriend send failed : " << str << endl;
    }
}
void groupchat(int cliendfd, string com)
{
    int idx = com.find(":");
    if (idx == -1) {
        cerr << "groupchat command invalid" << endl;
        return;
    }
    int groupid = atoi(com.substr(0, idx).c_str());
    string msg = com.substr(idx + 1, com.size() - idx);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["msg"] = msg;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;
    js["time"] = getCurrentTime();
    js["from"] = g_currentUser.getName();
    string str = js.dump();
    int len = send(cliendfd, str.c_str(), strlen(str.c_str()) + 1, 0);
    if (len == -1) {
        cerr << "groupchat send failed : " << str << endl;
    }
}
void loginout(int clientfd, string com)
{
    json js;
    js["msgid"] = LOGINOUT_MSG;
    js["id"] = g_currentUser.getId();
    string str = js.dump();
    int len = send(clientfd, str.c_str(), strlen(str.c_str()) + 1, 0);
    if (len == -1) {
        cerr << "loginout send failed : " << str << endl;
    } else {
        g_currentUserGroupList.clear();
        g_currentUserFriendList.clear();
        loginState = false;
    }
}