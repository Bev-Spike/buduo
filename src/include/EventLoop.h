#pragma once
#include <memory>
#include "Epoll.h"
class Channel;

//事件循环类，维护一个事件循环
class EventLoop {
  private:
    std::unique_ptr<Epoll> _ep;
    bool quit;

  public:
    EventLoop();
    ~EventLoop() = default;

    //开启事件循环
    void loop();
    //将事件添加到事件循环中
    void updateChannel(Channel*);
    void removeChannel(Channel*);
};