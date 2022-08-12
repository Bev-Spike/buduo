#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
class ThreadPool {
  private:
    std::vector<std::thread> _threads;
    //任务队列,当线程池运行的时候，线程会将任务队列的任务取出来执行
    std::queue<std::function<void()>> _tasks;
    std::mutex _taskMutex;
    std::condition_variable _cv;
    bool _stop;

  public:
    ThreadPool(int size = 4):_stop(false) {
        //可以用std::thread::hardware_concurrency()获取硬件支持线程数
        for (int i = 0; i < size; i++) {
            _threads.emplace_back(std::thread([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(_taskMutex);
                        _cv.wait(lock, [this]() {
                            //等待条件变量，当stop为true或任务列表不为空时返回
                            return _stop || ! _tasks.empty();
                        });
                        if (_stop && _tasks.empty())
                            return;
                        task = _tasks.front();
                        _tasks.pop();
                    }
                    //执行任务
                    task();
                }
            }));
        }
    }
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(_taskMutex);
            _stop = true;
        }
        _cv.notify_all();
        for (std::thread& th :_threads) {
            if (th.joinable()) {
                th.join();
            }
        }
    }
    void add(std::function<void()> func) {
        {
            std::unique_lock<std::mutex> lock(_taskMutex);
            if (_stop) {
                throw std::runtime_error("ThreadPoll already stop, can't add task any more");
            }
            _tasks.emplace(func);
        }
        _cv.notify_one();
    }
};