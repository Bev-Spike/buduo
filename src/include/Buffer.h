#pragma once
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

//参考了muduo的缓冲区设计

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode

class Buffer {
  public:
    //缓冲区最前面预留的空间
    //预留该空间是为了方便发送字节数的前插
    static const size_t kCheapPrepend = 8;
    //默认缓冲区的大小
    static const size_t kInitialSize = 1024;

  public:
    Buffer(size_t initialSize = kInitialSize);
    ssize_t readFd(int fd, int* savedErrno);
    
    //剩余可读空间
    size_t readableBytes() const;
    //剩余可写空间
    size_t writableBytes() const;
    //缓冲区头部可用空间
    size_t prependableBytes() const;

  
    //修改WriteIndex
    void hasWritten(size_t len);

    //向后修改readIndex指针，一般用于取出数据之后修改指针
    void retrieve(size_t len);
    //在取出所有数据后调用。将readIdx和writeIdx放回缓冲区的头部
    void retrieveAll();

    //将一定长度的数据作为String返回
    std::string retrieveAsString(size_t len);
    //取出缓冲区所有字符，作为String返回
    std::string retrieveAllAsString();

    //往缓冲区内写数据。如果空间不足，自动调整缓冲区的空间(包括整体前移和开辟新空间)
    void append(const char* data, size_t len);

    void append(const std::string& str);

    //保证缓冲区有足够的空间可写
    void ensureWritableBytes(size_t len);


    void print() {
        printf("size         :%zu\n", _buffer.size());
        printf("writeIdx     :%zu\n", _writerIndex);
        printf("readIdx      :%zu\n", _readerIndex);
        //打印缓存内数据
        std::string data = std::string(peek(), readableBytes());
        std::cout << data << std::endl;
    }
     //缓冲区可读内存的首地址
    char* peek();
  private:
    //缓冲区内存的首地址
    char* begin();
   
    //可写内存的首地址
    char* beginWrite();
    //如果剩余空间不足，开辟一块更大的新内存；如果剩余空间足够，将缓冲区的可读数据复制到缓冲区的头部
    void makeSpace(size_t len);
  private:
    std::vector<char> _buffer;
    size_t _readerIndex;
    size_t _writerIndex;

};