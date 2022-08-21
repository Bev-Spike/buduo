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
    //用户定义回调函数
    std::function<void(Connection*, Buffer*)> _messageCallback;
    //用户连接建立和关闭时调用
    std::function<void(Connection*)> _connetionCallback;
  public:
    Server(EventLoop*);
    ~Server() = default;
    void start() {
        _mainReactor->loop();
    }

    //设置用户自定义事件处理
    void setMessageCallback(std::function<void(Connection*, Buffer*)> fn);
    //该函数可用于连接建立和连接关闭时用户所作出的行为
    void setConnectionCallBack(std::function<void(Connection*)>);


    //自定义建立连接事件，由Acceptor调用
    void newConnction(Socket* clntSock);
    //自定义删除连接事件，由Connection调用
    void deleteConnection(Socket* clntSock);
};