#include "Buffer.h"
#include "Connection.h"
#include "Epoll.h"
#include "MyTypedef.h"
#include "Server.h"
#include "Socket.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Channel.h"
#include <cstdio>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <cstring>
#include <unordered_map>
using namespace std;


//简单的聊天室，发送任意一句消息都会向所有用户广播
class ChatServer {
  private:
    unique_ptr<Server> _server;
    unordered_map<int, ConnectionPTR> _connMap;
    int userCnt;
    mutex _mtx;
  public:
    ChatServer(EventLoop* loop):userCnt(0) {
        _server = make_unique<Server>(loop);
        _server->setConnectionCallBack(
            std::bind(&ChatServer::onConnection, this, placeholders::_1));
        _server->setMessageCallback(bind(
            &ChatServer::onMessage, this, placeholders::_1, placeholders::_2));
        
    }
    ~ChatServer() {}
    void start() {
        _server->start();
    }

  private:
    void onConnection(ConnectionPTR conn) {
        printf("Connection %d %s\n", conn->getSocket()->getFd(), conn->getState()== Connection::Connected? "up" : "down");

        unique_lock<mutex> lock(_mtx);
        if (conn->getState() == Connection::Connected) {
            _connMap[conn->getSocket()->getFd()] = conn;
            userCnt++;
        }
        else {
            _connMap.erase(conn->getSocket()->getFd());
            userCnt--;
        }
    }
    //遍历所有用户（自己除外），转发接收到的消息
    void onMessage(ConnectionPTR conn, Buffer* buffer) {
        string msg = buffer->retrieveAllAsString();
        cout << "收到"  << conn->getSocket()->getFd() << "的消息:" << msg << endl;
        for (auto& pair : _connMap) {
            ConnectionPTR user = pair.second;
            if (user != conn) {
                user->send(msg);
            }
        }
    }
};

int main() {
    EventLoop loop;
    ChatServer server(&loop);
    server.start();
}