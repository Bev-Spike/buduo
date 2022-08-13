#pragma once
#include <functional>
#include <bits/stdint-uintn.h>
#include <sys/epoll.h>
class Epoll;
class EventLoop;
//可以将channel理解为具体事件类
//一个Channel类自始至终只负责一个文件描述符，封装了对应的回调函数,对于
//不同的服务、不同的事件类型都可以在类中进行不同的处理
class Channel {
  private:
    EventLoop* _loop;
    int _fd;
    uint32_t _events; //当前channel注册的事件
    uint32_t _revents; // epoll wait返回时触发的事件
    bool _isInEpoll;
    //注册的回调函数，当事件触发时，会执行Channel注册的回调函数
    std::function<void()> _readCallBack;
    std::function<void()> _writeCallBack;
  public:
    Channel(EventLoop* loop, int _fd);
    ~Channel();

    int getFd();
    uint32_t getEvents();
    uint32_t getRevents();
    void setIsEt(bool);
    bool isInEpoll();
    void setInEpoll();
    void setNoInEpoll();
    //开启读事件
    void enableReading();
    void disableReading();
    void enableWriting();
    void disableWriting();
    //取消所有注册事件
    void disableAll();
    //在epoll中注销
    void remove();
    //判断是否正在监听写事件（是否已有数据待发送）
    bool isWriting() { return _events & EPOLLOUT; }
    bool isReading() {return _events & EPOLLIN;}
    //调用回调函数
    void handleEvent();
    void setRevents(uint32_t);
    void setReadCallBack(std::function<void()>);
    void setWriteCallBack(std::function<void()>);

};