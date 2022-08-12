#include "Socket.h"
#include "InetAddress.h"
#include <iostream>
#include <cstring>
#include "util.h"
using namespace std;
int main() {
    Socket* clntSock = new Socket();
    InetAddress servAddr("127.0.0.1", 8888);
    clntSock->connect(servAddr);
    while(true){
        char buf[1024];  //在这个版本，buf大小必须大于或等于服务器端buf大小，不然会出错，想想为什么？
        bzero(&buf, sizeof(buf));
        scanf("%s", buf);
        ssize_t write_bytes = write(clntSock->getFd(), buf, strlen(buf));
        if(write_bytes == -1){
            printf("socket already disconnected, can't write any more!\n");
            break;
        }
        bzero(&buf, sizeof(buf));
        ssize_t read_bytes = read(clntSock->getFd(), buf, sizeof(buf));
        if(read_bytes > 0){
            printf("message from server: %s\n", buf);
        }else if(read_bytes == 0){
            printf("server socket disconnected!\n");
            break;
        }else if(read_bytes == -1){

            errif(true, "socket read error");
            break;
        }
    }
    return 0;
}

