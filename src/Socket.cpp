#include "Socket.h"
#include "InetAddress.h"
#include "util.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>


Socket::Socket() : _fd(-1) {
    _fd = ::socket(AF_INET, SOCK_STREAM, 0);
    //设置端口可重用
    int val = 1;
    setsockopt(_fd,SOL_SOCKET,SO_REUSEADDR,(void *)&val,sizeof(int));
    errif(_fd== -1, "socket create error");
}

Socket::Socket(int fd) : _fd(fd) { errif(_fd == -1, "socket create error"); }

Socket::~Socket() {
    //socket fd的生命周期由Socket类来管理
    if (_fd != -1) {
        ::close(_fd);
        _fd = -1;
    }
}

void Socket::bind(InetAddress& addr) {
    //::bind函数不要求接管addr的生命周期，因此addr为局部变量也是可以的
    errif(::bind(_fd, (sockaddr*)&addr._addr, addr._addrlen) == -1, "socket bind error");
}

void Socket::listen() {
    errif(::listen(_fd, SOMAXCONN) == -1, "socket listen error");
}

void Socket::setNonBlocking() {
    fcntl(_fd, F_SETFL, fcntl(_fd, F_GETFL) | O_NONBLOCK);
}

void Socket::setBlocking() {
    fcntl(_fd, F_SETFL, fcntl(_fd, F_GETFL) & ~O_NONBLOCK);
}

int Socket::accept(InetAddress& addr) {
    int cilentfd = ::accept(_fd, (sockaddr*)&addr._addr, &addr._addrlen);
    errif(cilentfd == -1, "socket accept error");
    return cilentfd;
}

void Socket::connect(InetAddress& addr) {
    ::connect(_fd, (sockaddr*)&addr._addr, sizeof(addr._addr));
}

int Socket::getFd() const{
    return _fd;
}

