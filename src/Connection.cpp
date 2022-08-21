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
      _sock(sock),
      _connetionCallback([](Connection*){}){
    _channel = std::make_unique<Channel>(_loop, _sock->getFd());
    _readBuffer = std::make_unique<Buffer>();
    _writeBuffer = std::make_unique<Buffer>();
    std::function<void()> cb = std::bind(&Connection::handleRead, this);
    _channel->setReadCallBack(cb);
    _channel->setWriteCallBack(std::bind(&Connection::handleWrite, this));

}

void Connection::connectionEstablished() {

    //对于用户连接，统一使用边缘触发
    _channel->setIsEt(true);
    _channel->enableReading();
    _state = Connected;
    _connetionCallback(this);
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
            //printf("message from client fd %d: %s\n", sockfd, msg.c_str());
            write(sockfd, msg.c_str(), msg.length());
        }
        else if (bytes_read == -1 &&
                 errCode == EINTR) { //客户端正常中断、继续读取
            //printf("continue reading");
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

void Connection::setConnectionCallBack(std::function<void(Connection*)> cb) {
    _connetionCallback = cb;
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
    //printf("handleRead()\n");
    while (true) {
        ssize_t n = _readBuffer->readFd(_sock->getFd(), &savedErrno);
        //printf("read bytes %d\n", n);
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
    //printf("handleRead END \n");
    if (_state == Connected) {
       // printf("call user messagecallback\n");
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
    _connetionCallback(this);

    //在EpollLoop中移除channel
    _channel->remove();
    _deleteConnectionCallBack(_sock.get());
    //关闭socket
    _sock = nullptr;
}

//采用在本线程直接写的做法
void Connection::send(const std::string& msg) {
    send(msg.data(), msg.length());
}

//对于用户来讲，默认调用一次send，发送模块就会将数据完整的发送过去，不需要自己控制发送中的各种情况
void Connection::send(const char* data, ssize_t len) {
    if (_state == Connected) {
        if (_loop->isInLoopThread()) {
            sendInLoop(data, len);
        }
        //如果不在自己的IO线程运行，需要将其移动到自己的IO线程上运行
        else {
            void (Connection::*fp)(const std::string) =
                &Connection::sendInLoop;
            //跨进程传递数据需要将数据拷贝，因此再构造一个string作为参数。否则之后会自动析构数据对象导致内存访问出错。
            _loop->runInLoop(std::bind(fp, this, std::string(data, len)));
        }
    }
    else {
        printf("connection have closed\n");
    }
}

void Connection::sendInLoop(const std::string msg) {
    sendInLoop(msg.data(), msg.size());
}

//调用时应确保在自己的IO线程调用
void Connection::sendInLoop(const char* data, ssize_t len) {
    int haveWrite = 0;
    //如果当前发送缓冲区没有待发送的数据，直接向socket中写数据
    if (!_channel->isWriting() && _writeBuffer->readableBytes() == 0) {
        //因为使用的是ET触发，应一次性将所有数据写入，直到写完或写满
        while(haveWrite < len){
            int n = ::write(_sock->getFd(), data + haveWrite, len - haveWrite);
            //printf("write %d bytes!\n", n);
            if (n >= 0) {
                haveWrite += n;
                
            }
            //n == -1 通常是缓冲区已满
            else {
                if (errno != EWOULDBLOCK || errno != EAGAIN) {
                    _state = Closed;
                    printf("send error\n");
                }
            }
        }
    }
    //如果发送缓冲区有数据或者刚才的数据没发完，将data写入outBuffer，并注册写事件
    if (haveWrite < len) {
        _writeBuffer->append(data + haveWrite, len - haveWrite);
        _channel->enableWriting();
    }
}

//将socket上的数据读入到缓冲区中
void Connection::read() {}
