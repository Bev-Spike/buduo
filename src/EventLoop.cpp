#include "EventLoop.h"
#include "Epoll.h"
#include "Channel.h"
#include "util.h"
#include <bits/stdint-uintn.h>
#include <cstdio>
#include <memory>
#include <mutex>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <sys/eventfd.h>  //eventfd

EventLoop::EventLoop()
    : quit(false),
      _threadId(0),
      _callingPendingFunctors(false)
{
    _ep = std::make_unique<Epoll>();
    _wakeFd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    errif(_wakeFd < 0, "wakeFd creat error");
    _wakeupChannel = std::make_unique<Channel>(this, _wakeFd);
    _wakeupChannel->setReadCallBack(std::bind(&EventLoop::handleWakeup, this));
    _wakeupChannel->enableReading();
    _wakeupChannel->setInEpoll();

}

void EventLoop::loop() {
    //获取自己的线程ID
    _threadId = std::this_thread::get_id();
    while (!quit) {
        std::vector<Channel*> chs;
        chs = _ep->poll();
        //printf("poll return, chs.size=%d\n", (int)chs.size());
        for (auto& ch : chs) {
            //printf("ch.fd = %d\n", ch->getFd());
            //调用channel注册的回调函数
            ch->handleEvent();
        }

        doPendingQueue();
    }
}

void EventLoop::updateChannel(Channel* ch) { _ep->updateChannel(ch); }

void EventLoop::removeChannel(Channel* ch) { _ep->removeChannel(ch); }

void EventLoop::runInLoop(std::function<void()> cb) {
    if (isInLoopThread()) {
        cb();
    }
    else {
        queueInLoop(cb);
    }
}
void EventLoop::queueInLoop(std::function<void()> cb) {
    // 把任务加入到队列可能同时被多个线程调用，需要加锁
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _pendingQueue.push_back(cb);
    }
    //将cb放入队列后，还需要在必要的时候唤醒IO线程来处理
    // 1.如果当前不是loop线程，需要唤醒。如果是loop线程，说明正在处理事件，马上就能调用，无需唤醒
    // 2. 如果是在Loop线程调用该函数，但此时正在调用pending
    // function，需要唤醒，以下次调用
    if (!isInLoopThread() || _callingPendingFunctors) {
        wakeup();
    }
}
//该函数用于唤醒Epoll wait，以处理后续的pending function
void EventLoop::wakeup() {
    uint64_t one = 1;
    //往wakeupFd中写入8字节从而唤醒（eventFd的缓冲区也就8字节）
    ssize_t n = ::write(_wakeFd, &one, sizeof(one));
    //printf("have try to wakeup once\n");
    errif((n != sizeof(one)), "write wakeupFd error");
}
//执行pendingqueue里的所有函数
void EventLoop::doPendingQueue() {
    std::vector<std::function<void()>> functors;
    _callingPendingFunctors = true;

    {
        std::unique_lock<std::mutex> lock(_mutex);
        functors.swap(_pendingQueue);
    }
    for (auto i : functors) {
        i();
    }
    _callingPendingFunctors = false;
}
//用于处理wakeup事件
void EventLoop::handleWakeup() {
    //简单的读取数据即可
    uint64_t one = 1;
    ssize_t n = ::read(_wakeFd, &one, sizeof one);
    errif(n != sizeof one, "wakeupFd read error");
}

