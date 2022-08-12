#include "Epoll.h"
#include "Server.h"
#include "Socket.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Channel.h"
#include <cstdio>
#include <iostream>
#include <memory>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <cstring>
#include "Buffer.h"
using namespace std;



int main() {
    Buffer* buf = new Buffer();
    buf->print();
    return 0;
}

