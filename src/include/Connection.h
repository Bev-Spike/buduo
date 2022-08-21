#pragma once
#include "Buffer.h"
#include "EventLoop.h"
#include <functional>
#include <memory>
#include <string>
#include <sys/types.h>
#include "Socket.h"
#include "Channel.h"
class EventLoop;
class Buffer;
//连接类，用于负责一个连接，以及定义处理事件的逻辑
class Connection {
  public:
    enum State {
        Invalid = 1,
        Connected,
        Closed,
        Error,
    };
    
  private:
    EventLoop* _loop;
    std::unique_ptr<Socket> _sock;
    std::unique_ptr<Channel> _channel;
    std::unique_ptr<Buffer> _readBuffer;
    std::unique_ptr<Buffer> _writeBuffer;
    State _state;

    //删除连接的回调函数，由Server类定义
    std::function<void(Socket*)> _deleteConnectionCallBack;
    //定义接收到读事件之后的业务逻辑，由网络库的用户定义。
    std::function<void(Connection*, Buffer*)> _messageCallback;
    //定义连接建立与关闭时的业务逻辑
    std::function<void(Connection*)> _connetionCallback; 
  public:
    Connection(EventLoop* loop, Socket* sock);
    ~Connection();
    //连接刚建立时调用，一个连接仅调用一次
    void connectionEstablished();


    //自定义处理事件的逻辑
    void echo();
    void setDeleteConnctionCallBack(std::function<void(Socket*)>);

    void setMessageCallBack(std::function<void(Connection*, Buffer*)>);
    void setConnectionCallBack(std::function<void(Connection*)>);
    State getState();
    Socket* getSocket();
    void handleRead();
    void handleWrite();
    void handleClose();

    //采用在本线程直接写的做法
    //非阻塞写
    void send(const std::string& msg);
    void send(const char* data, ssize_t len);
    //由send调用，保证在本Epoll的线程内发送数据
    //避免不同线程读写统一socket的情况
    void sendInLoop(const std::string str);
    void sendInLoop(const char* data, ssize_t len);
    void read();

};