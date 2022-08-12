#include "Buffer.h"
#include <bits/types/struct_iovec.h>
#include <cstddef>
#include <iterator>
#include <string>
#include <sys/types.h>
#include <vector>
#include <algorithm>
#include <util.h>
#include <sys/uio.h>  // readv
const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;


Buffer::Buffer(size_t initialSize)
    : _buffer(kCheapPrepend + initialSize),
      _readerIndex(kCheapPrepend),
      _writerIndex(kCheapPrepend) {}

//从fd读取数据。
//适用于一次性读取短数据
ssize_t Buffer::readFd(int fd, int* savedErrno) {
    //开辟一段用于暂时存储缓存放不下的数据的空间
    char extraBuf[65535];
    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = begin() + _writerIndex;
    vec[0].iov_len = writable;
    vec[1].iov_base = extraBuf;
    vec[1].iov_len = sizeof(extraBuf);
    //如果缓冲区的空间足够大（128k，明明初始才1k），就不需要往栈内写入数据了
    int iovcnt = (writable < sizeof(extraBuf)) ? 2 : 1;
    ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0) {
        *savedErrno = errno;
    }
    //缓冲区空间足够的情况
    else if (n <= writable) {
        _writerIndex += n;
    }
    //要读入的数据大于缓冲区剩余空间的情况
    else {
        _writerIndex = _buffer.size();
        append(extraBuf, n - writable);
    }
    return n;
}


//剩余可读空间
size_t Buffer::readableBytes() const {
    return _writerIndex - _readerIndex;    
}
//剩余可写空间
size_t Buffer::writableBytes() const {
    return _buffer.size() - _writerIndex;    
}

//缓冲区头部可用空间
size_t Buffer::prependableBytes() const {
    return _readerIndex;    
}

  
    //修改WriteIndex
void Buffer::hasWritten(size_t len) {
    _writerIndex += len;    
}

//向后修改readIndex指针，一般用于取出数据之后修改指针
void Buffer::retrieve(size_t len) {
    errif(len > readableBytes(), "retrieve error");
    if (len < readableBytes()) {
        _readerIndex += len;
    }
    else {
        retrieveAll();
    }
}
//在取出所有数据后调用。将readIdx和writeIdx放回缓冲区的头部
void Buffer::retrieveAll() {
    _readerIndex = kCheapPrepend;
    _writerIndex = kCheapPrepend;
}

//将一定长度的数据作为String返回
std::string Buffer::retrieveAsString(size_t len) {
    errif(len > readableBytes(), "retrieve out error");
    std::string result(peek(), len);
    retrieve(len);
    return result;
}
//取出缓冲区所有字符，作为String返回
std::string Buffer::retrieveAllAsString() {
    return retrieveAsString(readableBytes());
}

//往缓冲区内写数据。如果空间不足，自动调整缓冲区的空间(包括整体前移和开辟新空间)
void Buffer::append(const char* data, size_t len) {
    ensureWritableBytes(len);
    std::copy(data, data + len, beginWrite());
    hasWritten(len);
}
void Buffer::append(const std::string& str) {
    append(str.data(), str.size());
}

//保证缓冲区有足够的空间可写
void Buffer::ensureWritableBytes(size_t len) {
    if (writableBytes() < len) {
        makeSpace(len);
    }
}

//缓冲区内存的首地址
char* Buffer::begin() {
    return &*_buffer.begin();
}
//缓冲区可读内存的首地址
char* Buffer::peek() {
    return begin() + _readerIndex;
}
//可写内存的首地址
char* Buffer::beginWrite() { return begin() + _writerIndex; }

//如果剩余空间不足，开辟一块更大的新内存；如果剩余空间足够，将缓冲区的可读数据复制到缓冲区的头部
void Buffer::makeSpace(size_t len) {
    if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
        _buffer.resize(_writerIndex + len);
    }
    else {
        errif(kCheapPrepend >= _readerIndex, "move buffer error");
        size_t readable = readableBytes();
        std::copy(begin() + _readerIndex,
                  begin() + writableBytes(),
                  begin() + kCheapPrepend);
        _readerIndex = kCheapPrepend;
        _writerIndex = _readerIndex + readable;
        errif(readable != readableBytes(), "after move buffer error");
    }
}