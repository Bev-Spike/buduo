#include "InetAddress.h"
#include <cstring>
InetAddress::InetAddress() : _addrlen(sizeof(_addr)){
    bzero(&_addr, sizeof(_addr));
}
InetAddress::InetAddress(const char* ip, uint16_t port) : _addrlen(sizeof(_addr)){
    bzero(&_addr, sizeof(_addr));
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = inet_addr(ip);
    _addr.sin_port = htons(port);
    _addrlen = sizeof(_addr);
}

InetAddress::~InetAddress(){
}