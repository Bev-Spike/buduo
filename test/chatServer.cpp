#include "Buffer.h"
#include "Connection.h"
#include "Epoll.h"
#include "Server.h"
#include "Socket.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Channel.h"
#include <cstdio>
#include <iostream>
#include <memory>
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
    unordered_map<int, Connection*> _connMap;


  public:
    
};