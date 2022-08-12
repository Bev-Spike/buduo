#pragma once
#include "InetAddress.h"
#include <unistd.h>
class InetAddress;
class Buffer;


class Socket {
  private:
    int _fd;

  public:
    //在该网络库中，socket fd的生命周期由Socket对象来管理，包括何时创建与关闭
    Socket();
    Socket(int);
    Socket(const Socket&) = delete;
    void operator= (const Socket&) = delete;
    ~Socket();

    void bind(InetAddress&);
    void listen();
    void setNonBlocking(); //设置为非阻塞socket
    void setBlocking();
    int accept(InetAddress&);
    void connect(InetAddress&);
    int getFd() const;

};