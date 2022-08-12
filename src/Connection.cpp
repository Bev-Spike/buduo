#include "Connection.h"

#include "Buffer.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"

#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <c++/7/bits/c++config.h>
#include <cstdio>
#include <functional>
#include <memory>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>

Connection::Connection(EventLoop* loop, Socket* sock)
    : _loop(loop),
      _sock(sock) {
    _channel = std::make_unique<Channel>(_loop, _sock->getFd());
    _readBuffer = std::make_unique<Buffer>();
    _writeBuffer = std::make_unique<Buffer>();
    std::function<void()> cb = std::bind(&Connection::handleRead, this);
    _channel->setReadCallBack(cb);
    _channel->setWriteCallBack(std::bind(&Connection::handleWrite, this));
    //对于用户连接，统一使用边缘触发
    _channel->setIsEt(true);
    _channel->enableReading();
    _state = Connected;
}
Connection::~Connection() { printf("Connction 析构\n"); }
void Connection::echo() {
    int sockfd = _sock->getFd();
    int errCode;
    while (
        true) { //由于使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
        ssize_t bytes_read = _readBuffer->readFd(sockfd, &errCode);
        //_readBuffer->print();
        if (bytes_read > 0) {
            std::string msg = _readBuffer->retrieveAllAsString();
            printf("message from client fd %d: %s\n", sockfd, msg.c_str());
            write(sockfd, msg.c_str(), msg.length());
        }
        else if (bytes_read == -1 &&
                 errCode == EINTR) { //客户端正常中断、继续读取
            printf("continue reading");
            continue;
        }
        else if (bytes_read == -1 &&
                 ((errCode == EAGAIN) ||
                  (errCode ==
                   EWOULDBLOCK))) { //非阻塞IO，这个条件表示数据全部读取完毕
            printf("finish reading once, errno: %d\n", errCode);
            break;
        }
        else if (bytes_read == 0) { // EOF，客户端断开连接
            printf("EOF, client fd %d disconnected\n", sockfd);
            handleClose();
            break;
        }
    }
}

void Connection::setDeleteConnctionCallBack(std::function<void(Socket*)> cb) {
    _deleteConnectionCallBack = cb;
}

void Connection::setMessageCallBack(std::function<void(Connection*, Buffer*)> callback) {
    _messageCallback = callback;
}

Connection::State Connection::getState() {
    return _state;
}
Socket* Connection::getSocket() {
    return  _sock.get();
}
void Connection::handleRead() {
    //先读取所有数据到Buffer中，再调用用户注册的回调函数
    int savedErrno;
    while (true) {
        ssize_t n = _readBuffer->readFd(_sock->getFd(), &savedErrno);
        if (n == 0) {
            printf("read EOF, client fd %d disconnected\n", _sock->getFd());
            handleClose();
            break;
        }
        else if (n == -1) {
            if (savedErrno == EINTR)
                continue;
            if (((savedErrno == EAGAIN) ||
                 (savedErrno ==
                  EWOULDBLOCK))) { // 非阻塞IO，这个条件表示数据全部读取完毕
                break;
            }
        }
    }
    if(_state == Connected){
        _messageCallback(this, _readBuffer.get());
    }
}

//将缓冲区的数据写入socket中
void Connection::handleWrite() {
    //因为采用ET模式，需要循环将数据全部发送出去，直到返回EAGAIN
    while (true) {
        ssize_t n = ::write(_sock->getFd(),
                            _writeBuffer->peek(),
                            _writeBuffer->readableBytes());
        if (n >= 0) {
            _writeBuffer->retrieve(n);
            if (_writeBuffer->readableBytes() == 0) {
                _channel->disableWriting();
                break;
            }
        }
        if (n == -1) {
            if (errno == EINTR)
                continue;
            else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            else {
                printf("Other error on client fd %d\n", _sock->getFd());
                _state = State::Closed;
                break;
            }
        }
    }
}

//采用延迟关闭的做法。并不直接析构connection对象，仅仅关闭socket和注册的channel，等到下一次相同的
// socket fd出现时，自动将当前的connection对象覆盖并析构掉
void Connection::handleClose() {
    _state = Closed;
    //在EpollLoop中移除channel
    _channel->remove();
    _deleteConnectionCallBack(_sock.get());
    //关闭socket
    _sock = nullptr;
}

//采用在本线程直接写的做法
void Connection::send(std::string msg) { send(msg.data(), msg.length()); }
//对于用户来讲，默认调用一次send，发送模块就会将数据完整的发送过去，不需要自己控制发送中的各种情况
void Connection::send(char* data, ssize_t len) {
    if (_state == Connected) {
        int haveWrite = 0;
        //如果当前发送缓冲区没有待发送的数据，直接向socket中写数据
        if (_writeBuffer->readableBytes() == 0) {
            int n = ::write(_sock->getFd(), data, len);
            if (n >= 0) {
                haveWrite += n;
            }
            else {
                haveWrite = 0;
                if (errno != EWOULDBLOCK || errno != EAGAIN) {
                    _state = Closed;
                    printf("send error\n");
                }
            }
        }
        //如果发送缓冲区有数据或者刚才的数据没发完，将data写入outBuffer，并注册写事件
        if (haveWrite < len) {
            _writeBuffer->append(data + haveWrite, len - haveWrite);
            _channel->enableWriting();
        }
    }
    else {
        printf("connection have closed\n");
    }
}
//将socket上的数据读入到缓冲区中
void Connection::read() {}
