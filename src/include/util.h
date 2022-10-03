#pragma once

//如果系统调用发生错误，则将错误信息输出到终端中
#include <string>
void errif(bool, const char*);


class TimeStamp {};

std::string getTidStr();