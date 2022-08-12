#pragma once
#include <functional>
#include <memory>
class Socket;
class EventLoop;
class Channel;
class InetAddress;

//专门用于接收连接的类，包括创建监听socket，bind和listen
class Acceptor {
  private:
    std::unique_ptr<Socket> _sock;
    std::unique_ptr<InetAddress> _addr;
    std::unique_ptr<Channel> _channel;
    EventLoop* _loop;

  public:
    Acceptor(EventLoop* loop);
    ~Acceptor() = default;
    void acceptConnection();
    //回调函数，为接收连接的方式，由服务器核心类Server来提供
    std::function<void(Socket*)> _newConnctionCallBack;

    void setNewConnectionCallBack(std::function<void(Socket*)>);
};