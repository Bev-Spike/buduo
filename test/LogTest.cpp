#include "Logger.h"
#include <string>
#include <iostream>
using namespace std;


int main() {
    LOG_INFO << "this is a INFO"
             << "中文" << 1 << ' ' << 1.2345 << '\n';
    LOG_TRACE << "this is a trace";
    LOG_WARN << "this is a warm";
    LOG_DEBUG << "this is a debug";
    LOG_ERROR << "this is a error";
    //LOG_FATAL << "this is a Fatal";
    // LOG_SYSFATAL << "this is a SYSFATAL";
    // LOG_SYSERR << "this is a SYSERR";
    
    return 0;
}