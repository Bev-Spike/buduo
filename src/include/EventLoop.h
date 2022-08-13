#pragma once
#include <memory>
#include <mutex>
#include <queue>
#include "Epoll.h"
#include <functional>
#include <thread>
class Channel;

//事件循环类，维护一个事件循环
class EventLoop {
  private:
    std::unique_ptr<Epoll> _ep;
    bool quit;
    int _wakeFd;
    //当前线程ID
    std::thread::id _threadId;
    std::unique_ptr<Channel> _wakeupChannel;
    //需要在本线程执行的函数
    std::vector<std::function<void()>> _pendingQueue;
    bool _callingPendingFunctors;//原子变量
    std::mutex _mutex;
  public:
    EventLoop();
    ~EventLoop() = default;

    //开启事件循环
    void loop();
    //将事件添加到事件循环中
    void updateChannel(Channel*);
    void removeChannel(Channel*);

    void runInLoop(std::function<void()>);
  //判断当前是否在本Loop的线程
    bool isInLoopThread() { return std::this_thread::get_id() == _threadId; }
  private:
    void queueInLoop(std::function<void()>);
    void wakeup();
    //执行pendingqueue里的所有函数
    void doPendingQueue();
    //用于处理wakeup事件
    void handleWakeup();

   
};