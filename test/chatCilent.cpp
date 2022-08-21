#include "Buffer.h"
#include "Connection.h"
#include "Epoll.h"
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
#include <thread>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <cstring>
using namespace std;


class ChatCilent {
  private:
    Connection _conn;
    EventLoop* _loop;
    Socket* _sock;

  public:
    ChatCilent(EventLoop* loop, Socket* sock) : _conn(loop, sock) , _loop(loop), _sock(sock){
        ///
        //目前使用的网络库需要非阻塞的Socket
        _sock->setNonBlocking();
        _conn.setMessageCallBack(bind(
            &ChatCilent::onMessage, this, placeholders::_1, placeholders::_2));
        _conn.connectionEstablished();
    }

    //读到消息后的行为
    void onMessage(Connection* conn, Buffer* buf) {
        string msg = buf->retrieveAllAsString();
        cout << ">>>" << msg << endl;
    }
    void connect(InetAddress& addr) { _sock->connect(addr); }

    void send(string& msg) {
        _conn.send(msg);
    }
};

int main() {
    EventLoop* loop = new EventLoop();
    Socket* clntSock = new Socket();
    InetAddress servAddr("127.0.0.1", 8888);
    ChatCilent cilent(loop, clntSock);
    cilent.connect(servAddr);
    thread woker([&cilent] {
        while (1) {
            string msg;
            cin >> msg;
            cilent.send(msg);
            cout << "<<" <<  msg << endl;
        }
    });
    loop->loop();
    
}