#pragma once
#include <arpa/inet.h>
#include <bits/stdint-uintn.h>
#include <netinet/in.h>
#include <sys/socket.h>

//Socket地址封装
class InetAddress {
  public:
    struct sockaddr_in _addr;
    socklen_t _addrlen;
    InetAddress();
    InetAddress(const char* ip, uint16_t port);
    ~InetAddress();
};