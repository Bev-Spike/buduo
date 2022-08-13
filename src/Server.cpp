#include "Server.h"
#include "Connection.h"
#include "EventLoop.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "Acceptor.h"
#include "ThreadPool.h"
#include <algorithm>
#include <functional>
#include <memory>
#include <string.h>
#include <thread>
#include <unistd.h>

Server::Server(EventLoop* loop) : _mainReactor(loop) {
    _acceptor = std::make_unique<Acceptor>(_mainReactor);
    std::function<void(Socket*)> cb =
        std::bind(&Server::newConnction, this, std::placeholders::_1);
    _acceptor->setNewConnectionCallBack(cb);

    //自动获取硬件支持的线程数,同时也是subReactor的数量
    int size = std::thread::hardware_concurrency();
    _threadPool = std::make_unique<ThreadPool>(size);
    //创建并开启事件循环
    for (int i = 0; i < size; i++) {
        _subReactors.push_back(std::unique_ptr<EventLoop>(new EventLoop()));
        std::function<void()> subLoop =
            std::bind(&EventLoop::loop, _subReactors[i].get());
        _threadPool->add(subLoop);
        printf("subReactor %d 创建成功\n", i);
    }
}



//新连接的建立由Server类定义，由acceptor调用
//由Acceptor传入新连接的客户Socket，将其封装为Connection类保存在map中
void Server::newConnction(Socket* clntSock) {
    //使用完全随机调度策略，将该连接socket描述符添加到一个subReactor中。
    int random = clntSock->getFd() % _subReactors.size();
    std::unique_ptr<Connection> conn(new Connection(_subReactors[random].get(), clntSock));
    //设置删除连接的回调函数，当客户任务结束时调用
    std::function<void(Socket*)> cb =
        std::bind(&Server::deleteConnection, this, std::placeholders::_1);
    conn->setDeleteConnctionCallBack(cb);
    conn->setMessageCallBack(_messageCallback);
    conn->setConnectionCallBack(_connetionCallback);
    conn->connectionEstablished();
    connections[clntSock->getFd()] = move(conn);
}

void Server::deleteConnection(Socket* clntSock) {
    //由于采用的是延迟析构的做法，Connection对象内部对象的处理交给他自己，Server这边并不需要做什么
    printf("deleteConnetion socket fd:%d \n", clntSock->getFd());
}

void Server::setMessageCallback(std::function<void(Connection*, Buffer*)> fn) {
    _messageCallback = std::move(fn);
}


void Server::setConnectionCallBack(std::function<void(Connection*)> cb) {
    _connetionCallback = std::move(cb);
}