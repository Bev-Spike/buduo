#include "Logger.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <pthread.h>
#include <string>
#include <sys/stat.h>
#include <sys/time.h>
#include <thread>
#include <time.h>
#include <unistd.h>
#include "util.h"

const char* LogLevelName[6] = {
    "[TRACE] ",
    "[DEBUG] ",
    "[INFO]  ",
    "[WARN]  ",
    "[ERROR] ",
    "[FATAL] ",
};

LogStream::LogStream() { _buffer.reserve(faultSize); }

LogStream& LogStream::operator<<(bool v) {
    _buffer.append(v ? "1" : "0", 1);
    return *this;
}

LogStream& LogStream::operator<<(short i) {
    _buffer.append(std::to_string(i));
    return *this;
}
LogStream& LogStream::operator<<(unsigned short i) {
    _buffer.append(std::to_string(i));
    return *this;
}
LogStream& LogStream::operator<<(int i) {
    _buffer.append(std::to_string(i));
    return *this;
}
LogStream& LogStream::operator<<(unsigned int i) {
    _buffer.append(std::to_string(i));
    return *this;
}
LogStream& LogStream::operator<<(long i) {
    _buffer.append(std::to_string(i));
    return *this;
}
LogStream& LogStream::operator<<(unsigned long i) {
    _buffer.append(std::to_string(i));
    return *this;
}
LogStream& LogStream::operator<<(long long i) {
    _buffer.append(std::to_string(i));
    return *this;
}
LogStream& LogStream::operator<<(unsigned long long i) {
    _buffer.append(std::to_string(i));
    return *this;
}
LogStream& LogStream::operator<<(double i) {
    _buffer.append(std::to_string(i));
    return *this;
}
LogStream& LogStream::operator<<(float i) {
    _buffer.append(std::to_string(i));
    return *this;
}

LogStream& LogStream::operator<<(char i) {
    _buffer += i;
    return *this;
}

LogStream& LogStream::operator<<(const char* str) {
    auto len = std::strlen(str);
    _buffer.append(str, len);
    return *this;
}
LogStream& LogStream::operator<<(const std::string& str) {
    _buffer.append(str);
    return *this;
}

void defaultOutput(const char* msg, int len)
{
  size_t n = fwrite(msg, 1, len, stdout);
}
void defaultFlush()
{
  fflush(stdout);
}
Logger::LogLevel g_logLevel = Logger::LogLevel::TRACE;
Logger::OutPutFuc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;

// SYS_ERROR 或 SYS_FATAL
Logger::Logger(const char* sourseFileName, int line, bool isAbort)
    : _sourseFile(strrchr(sourseFileName, '/')),
      _level(isAbort ? FATAL : ERROR),
      _line(line),
      _func("")
      {
    impl(isAbort?FATAL:ERROR, errno, sourseFileName);
}
Logger::Logger(const char* sourseFileName, int line, LogLevel level)
    : _sourseFile(strrchr(sourseFileName, '/')),
      _level(level),
      _line(line),
      _func("") {
    impl(level, 0, sourseFileName);
}
Logger::Logger(const char* sourseFileName,
               int line,
               LogLevel level,
               const char* func)
    : _sourseFile(strrchr(sourseFileName, '/')),
      _level(level),
      _line(line),
      _func(func) {
    impl(level, 0, sourseFileName);
}

Logger::~Logger() {
    _stream << " - " << _sourseFile << " : " << _line << " " << _func << '\n';
    const std::string buffer = _stream.getBuffer();
    g_output(buffer.data(), buffer.length());
    if (_level == FATAL) {//出现指明错误则直接结束进程
        g_flush();
        abort();
    }
}
//返回当前系统的loglevel
Logger::LogLevel Logger::logLevel() {
    return g_logLevel;
}

void Logger::setOutPut(OutPutFuc func) {
    g_output = func;
}
void Logger::setFlush(FlushFunc func) {
    g_flush = func;
}
void Logger::setLogLevel(LogLevel level) {
    g_logLevel = level;
}

//格式化时间，并将时间写入流中
void Logger::formatTime() {
    struct timeval now = {0, 0};
    gettimeofday(&now, NULL);
    time_t t = now.tv_sec;
    struct tm* sys_tm = localtime(&t);
    char timeStr[64];
    int len = snprintf(timeStr,
                       sizeof(timeStr),
                       "%4d%02d%02d %02d:%02d:%02d",
                       sys_tm->tm_year + 1900,
                       sys_tm->tm_mon + 1,
                       sys_tm->tm_mday,
                       sys_tm->tm_hour,
                       sys_tm->tm_min,
                       sys_tm->tm_sec);
    _stream << timeStr;
}
//提供统一的组装log字符串的方法
// 日期     时间     线程  级别  正文      源文件名  行号
// 20221002 20:43:13 23251 INFO 正文内容 - test.cpp:118
void Logger::impl(LogLevel level, int savedErrno, const char* fileName) {
    formatTime();
    _stream << " " << getTidStr();
    _stream << " " << LogLevelName[level];
    if (savedErrno != 0) {
        _stream << strerror(savedErrno) << " (errno=" << savedErrno << ") ";
    }
}
