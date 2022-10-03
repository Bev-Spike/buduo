#include "Buffer.h"
#include "Connection.h"
#include "Epoll.h"
#include "Server.h"
#include "Socket.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Channel.h"
#include "Logger.h"
#include <cstdio>
#include <iostream>
#include <memory>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <cstring>
using namespace std;


void echo(Connection* conn, Buffer* readBuf) {
    string msg = readBuf->retrieveAllAsString();

    LOG_INFO << "Message from client " << conn->getSocket()->getFd() << ": "
              << msg;
    conn->send(msg);
}

int main() {
    unique_ptr<EventLoop> loop(new EventLoop());
    unique_ptr<Server> server(new Server(loop.get()));
    server->setMessageCallback(echo);
    server->setConnectionCallBack([](Connection* conn) {
        if(conn->getState() == Connection::Connected){
            LOG_INFO << "hello new Connetion :" << conn->getSocket()->getFd();
        }
        else {
            LOG_INFO << "bye Connetion";
        }
    });
    loop->loop();

    return 0;
}

