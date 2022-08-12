// 模拟大量高并发请求

#include "Buffer.h"
#include "InetAddress.h"
#include "Socket.h"
#include "ThreadPool.h"
#include "util.h"

#include <cstring>
#include <functional>
#include <iostream>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

//模拟一个客户端的行为
void oneCilent(int msgCnt) {
    Socket sock;
    InetAddress addr("127.0.0.1", 8888);
    sock.connect(addr);

    int sockfd = sock.getFd();
    //Buffer* sendBuffer = new Buffer();
    Buffer* readBuffer = new Buffer();

    char sendMsg[1024] = "I am Cilent!!!!!!!!!!!!!!!!!!!!!";
    for (int i = 0; i < msgCnt; i++) {
        ssize_t writeBytes = write(sockfd, sendMsg, strlen(sendMsg));
        if (writeBytes == -1) {
            printf("socket already disconnected, can't write any more!\n");
            break;
        }

        int haveRead = 0;
        while (true) {
            int errorCode;
            int readBytes = readBuffer->readFd(sockfd, &errorCode);
            if (readBytes > 0) {
                haveRead+= readBytes;
            }
            else if (readBytes == 0) {
                printf("server disconnected!\n");
                exit(EXIT_SUCCESS);
            }
            //该判断条件仅适用于服务器回传字符串
            if (haveRead >= writeBytes) {
                printf("count: %d, message from server: %s\n", i, readBuffer->retrieveAllAsString().c_str());
                break;
            }
        }
    }
    delete readBuffer;
}

int main() {
    //开启的线程数
    int threadsCnt = 100;
    //每个线程发送的消息数
    int msgCnt = 100;
    ThreadPool* poll = new ThreadPool(threadsCnt);
    function<void()> func = std::bind(oneCilent, msgCnt);
    for (int i = 0; i < threadsCnt; i++) {
        poll->add(func);
    }

    delete poll;
    return 0;
}