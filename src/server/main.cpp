#include "chatserver.hpp"

#include "chatservice.hpp"
#include <iostream>
#include <signal.h>
using namespace std;

void resetHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        cerr << "ip/port is not right example : ./chatclient 127.0.0.1 10000" << endl;
        exit(-1);
    }
    char* ip = argv[1];
    uint16_t port = atoi(argv[2]);

    signal(SIGINT, resetHandler);
    EventLoop baseloop;
    InetAddress addr(ip, port);
    ChatServer server(&baseloop, addr, "ChatServer");
    server.start();
    baseloop.loop();


    return 0;
}