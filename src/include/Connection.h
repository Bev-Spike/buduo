#pragma once
#include "Buffer.h"
#include "EventLoop.h"
#include <functional>
#include <memory>
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
  public:
    Connection(EventLoop* loop, Socket* sock);
    ~Connection();

    //自定义处理事件的逻辑
    void echo();
    void setDeleteConnctionCallBack(std::function<void(Socket*)>);

    void setMessageCallBack(std::function<void(Connection*, Buffer*)>);
    State getState();
    Socket* getSocket();
    void handleRead();
    void handleWrite();
    void handleClose();

    //采用在本线程直接写的做法
    void send(std::string msg);
    void send(char* data, ssize_t len);

    void read();

};