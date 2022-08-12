#include "Acceptor.h"
#include "Socket.h"
#include "Channel.h"
#include "Server.h"
#include "InetAddress.h"
#include <functional>
#include <memory>
#include <sched.h>

Acceptor::Acceptor(EventLoop* loop) : _loop(loop) {
    _sock = std::make_unique<Socket>();
    _addr = std::make_unique<InetAddress>("127.0.0.1", 8888);
    _sock->bind(*_addr);
    //监听socket使用阻塞IO比较好
    _sock->setBlocking();
    _sock->listen();
    

    _channel = std::make_unique<Channel>(loop, _sock->getFd());
    std::function<void()> cb = std::bind(&Acceptor::acceptConnection, this);
    _channel->setReadCallBack(cb);
    //设置为电平触发比较合适
    _channel->setIsEt(false);
    _channel->enableReading();
}

void Acceptor::acceptConnection() {
    InetAddress clntAddr;
    Socket* clntSock = new Socket(_sock->accept(clntAddr));
    printf("new client fd %d! IP: %s Port: %d\n",
           clntSock->getFd(),
           inet_ntoa(clntAddr._addr.sin_addr),
           ntohs(clntAddr._addr.sin_port));
    clntSock->setNonBlocking();
    _newConnctionCallBack(clntSock);
}

void Acceptor::setNewConnectionCallBack(std::function<void(Socket*)> cb) {
    _newConnctionCallBack = cb;
}