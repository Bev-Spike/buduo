#include "util.h"
#include <cstdio>
#include <cstdlib>

void errif(bool condition, const char* errmsg) {
    if (condition) {
        perror(errmsg);
        exit(EXIT_FAILURE);
    }
}

std::string getTidStr() {
    std::string tid = std::to_string((unsigned long)pthread_self());
    return tid.substr(tid.length() - 5);
}