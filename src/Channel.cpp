#include "Channel.h"
#include "Epoll.h"
#include "EventLoop.h"
#include <bits/stdint-uintn.h>
#include <cstdio>
#include <sys/epoll.h>


Channel::Channel(EventLoop* loop, int fd)
    : _loop(loop),
      _fd(fd),
      _events(0),
      _revents(0),
      _isInEpoll(false){};
Channel::~Channel() {
   // printf("channel 析构\n");
}

void Channel::setIsEt(bool isET) {
    if (isET) {
        _events |= EPOLLET;
    }
    else {
        _events &= ~EPOLLET;
    }
}

void Channel::enableReading() {
    _events |= EPOLLIN;
    _loop->updateChannel(this);
}

void Channel::enableWriting() {
    _events |= EPOLLOUT;
    _loop->updateChannel(this);
}

int Channel::getFd() { return _fd; }

uint32_t Channel::getEvents() { return _events; }

uint32_t Channel::getRevents() { return _revents; }

bool Channel::isInEpoll(){return _isInEpoll;};

void Channel::setInEpoll() { _isInEpoll = true; }

void Channel::setNoInEpoll(){_isInEpoll = false;};

void Channel::setRevents(uint32_t ev) { _revents = ev; }

void Channel::setReadCallBack(std::function<void()> callback) {
    _readCallBack = callback;
}

void Channel::setWriteCallBack(std::function<void()> callback) {
    _writeCallBack = callback;
}

void Channel::handleEvent() {
    //读事件、带外事件
    if (_revents & (EPOLLIN | EPOLLPRI)) {
        //printf("read callback\n");
        _readCallBack();
    }
    //写事件触发
    else if (_revents & (EPOLLOUT)) {
        //printf("write callback\n");
        _writeCallBack();
    }
    else {
        printf("This event cant be handle: %d\n", _revents);
    }
}

void Channel::disableReading() {
    _events &= ~EPOLLIN;
    _loop->updateChannel(this);
}

void Channel::disableWriting() {
    _events &= ~EPOLLOUT;
    _loop->updateChannel(this);
}

void Channel::disableAll() {
     _events &= 0;
    _loop->updateChannel(this);
}

void Channel::remove() {
    _loop->removeChannel(this);
}