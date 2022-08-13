#include "Epoll.h"
#include "util.h"
#include "Channel.h"
#include <bits/stdint-uintn.h>
#include <cstring>
#include <strings.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

//创建一个epoll对象
Epoll::Epoll() : _epfd(-1) {
    _epfd = epoll_create1(0);
    errif(_epfd == -1, "epoll create error");
    memset(_events, 0, sizeof(_events));
}

Epoll::~Epoll() {
    if (_epfd != -1) {
        close(_epfd);
        _epfd = -1;
    }
}

void Epoll::addFd(int fd, uint32_t op) {
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.fd = fd;
    ev.events = op;
    errif(epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add error");
}


//封装epoll_wait，将返回的事件封装为channel数组
std::vector<Channel*> Epoll::poll(int timeout) {
    std::vector<Channel*> activeChannels;
    int nfds = epoll_wait(_epfd, _events, MAX_EVENTS, timeout);
    //errif(nfds == -1, "epoll wait error");
    for (int i = 0; i < nfds; i++) {
        Channel* ch = (Channel*)_events[i].data.ptr;
        ch->setRevents(_events[i].events);
        activeChannels.push_back(ch);
    }
    //C++11中，返回局部变量优先使用移动构造
    return activeChannels;
}

void Epoll::updateChannel(Channel* channel) {
    int fd = channel->getFd();
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.ptr = channel;
    ev.events = channel->getEvents();
    if (!channel->isInEpoll()) {
        errif(epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &ev) == -1,
              "epoll add error");
        channel->setInEpoll();
    }
    else {
        errif(epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &ev) == -1,
              "epoll modify error");
        }
}

void Epoll::removeChannel(Channel* ch) {
    int fd = ch->getFd();
    errif(epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, nullptr) == -1,
          "epoll del error");
    ch->setNoInEpoll();
}