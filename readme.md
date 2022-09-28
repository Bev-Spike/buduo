# buduo网络库
## 简介
本网络库吸收了muduo库的部分设计思想，经过个人的整理与“简化”，设计与实现了一个基于主从Reactor模型的网络库，设计思想仍然沿用muduo的"one loop per thread"。

## 相关知识点
1. EPOLL IO多路复用 
2. 主从Reactor模型+非阻塞IO
3. 多线程高并发、线程池
4. 读写缓冲区
5. 基于事件回调的更加通用的网络库，使用相当简单
6. 使用C++11特性，如智能指针、function等

## 程序框架
待补充

## 调用示例
见test/server
```
//简单的echo服务器实现
void echo(Connection* conn, Buffer* readBuf) {
    string msg = readBuf->retrieveAllAsString();
    std::cout << "Message from client " << conn->getSocket()->getFd() << ": "
              << msg << std::endl;

    conn->send(msg);
}

int main() {
    unique_ptr<EventLoop> loop(new EventLoop());
    unique_ptr<Server> server(new Server(loop.get()));
    server->onMessage(echo);
    loop->loop();

    return 0;
}

```


## 小小的改动
本人是基于学习的心态写出了这整个网络库。虽然整个网络库透露着阉割版muduo库的味道，但一些小细节还是有个人的一些思考的。<br>
例如：muduo库关闭连接时使用了若干个回调函数，并切换两次线程，甚至为安全析构对象专门设置了一个pendingQueue。虽然这样完美实现了对象的安全析构，但是仅仅是为了关闭连接而设计的这一连串复杂的操作对学习与维护来说是相当不友好的。<br>
本人以牺牲一点点性能为代价，基于延迟删除的思想，将这一连串操作简化为在被动关闭连接的时候仅仅注销channel和关闭socket fd。这样会留下一个没有用的Connection对象放在Server对象的map中，直到下次有相同的socket fd，初始化一个新的Connection对象后将map中原有的Connection 对象取代并析构。<br>
这样同样可以做到安全析构对象，不会导致对象在使用的期间惨遭析构。唯一的缺点是内存不能及时释放，但总的时间效率和内存效率应该是不会差太多的。

## to do
1. 定时器
2. 其他更加人性化的成员函数
3. 网络库自带的日志系统
4. 错误处理，目前遇到各种系统问题的时候并不能很好的处理这些错误。

## 特别鸣谢
陈硕 [muduo](https://github.com/chenshuo/muduo)<br>
yuesong-feng [三十天服务器系列](https://github.com/yuesong-feng/30dayMakeCppServer)

## 更新日志

*2022.9.28*<br>
修复了BUG：连接销毁可能会出现的问题：如果关闭socket的时候马上就有新socket进来，就会马上析构连接对象，导致的内存安全问题。  
具体方式：将server中的Connection类中的unique_ptr换成了shared_ptr,使用共享指针管理Connection对象的生命周期。同时，将Connection类继承enable_shared_from_this,在调用handleClose的时候，调用shared_from_this， 生成一个智能指针保证函数可以完整执行结束。