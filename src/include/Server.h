#pragma once
#include "Acceptor.h"
#include "Buffer.h"
#include "Connection.h"
#include "ThreadPool.h"
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

class EventLoop;
class Socket;
class Connection;
class Server {
  private:
    //主reactor
    EventLoop* _mainReactor;

    //网络组件的生命周期由Server核心类来管理
    //从reactor
    std::vector<std::unique_ptr<EventLoop>> _subReactors;
    //线程池
    std::unique_ptr<ThreadPool> _threadPool;
    std::unique_ptr<Acceptor> _acceptor;
    std::unordered_map<int, std::unique_ptr<Connection>> connections;

    std::function<void(Connection*, Buffer*)> _messageCallback;
  public:
    Server(EventLoop*);
    ~Server() = default;
    //设置用户自定义事件处理
    void onMessage(std::function<void(Connection *, Buffer*)> fn);
    //自定义读事件
    void handleReadEvent(int);

    //自定义建立连接事件，由Acceptor调用
    void newConnction(Socket* clntSock);
    //自定义删除连接事件，由Connection调用
    void deleteConnection(Socket* clntSock);
};