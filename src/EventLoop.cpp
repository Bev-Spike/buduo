#include "EventLoop.h"
#include "Epoll.h"
#include "Channel.h"
#include <memory>
#include <vector>

EventLoop::EventLoop() : quit(false) {
    _ep = std::make_unique<Epoll>();
}

void EventLoop::loop() {
    while (!quit) {
        std::vector<Channel*> chs;
        chs = _ep->poll();
        for (auto& ch : chs) {
            //调用channel注册的回调函数
            ch->handleEvent();
        }
    }
}

void EventLoop::updateChannel(Channel* ch) { _ep->updateChannel(ch); }

void EventLoop::removeChannel(Channel* ch) {
    _ep->removeChannel(ch);
}