#pragma once
#include <vector>
#include <sys/epoll.h>

#define MAX_EVENTS 1000

class Channel;
class Epoll {
  private:
    int _epfd;
    struct epoll_event _events[MAX_EVENTS]; //用于存放EPOLL事件

  public:
    Epoll();
    ~Epoll();

    //将文件描述符添加到EPOLL监听中
    void addFd(int fd, uint32_t op);
    void updateChannel(Channel*);
    void removeChannel(Channel*);
    std::vector<Channel*> poll(int timeout = -1);
};