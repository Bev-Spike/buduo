#pragma once
#include <functional>
#include <string>

//用于存放单条流式LOG
class LogStream {
  private:
    std::string _buffer;
    static const int faultSize = 256;

  public:
    LogStream();
    std::string getBuffer(){return _buffer;}
    LogStream& operator<<(bool v);

    LogStream& operator<<(short);
    LogStream& operator<<(unsigned short);
    LogStream& operator<<(int);
    LogStream& operator<<(unsigned int);
    LogStream& operator<<(long);
    LogStream& operator<<(unsigned long);
    LogStream& operator<<(long long);
    LogStream& operator<<(unsigned long long);
    LogStream& operator<<(double);
    LogStream& operator<<(float);
    LogStream& operator<<(char);

    LogStream& operator<<(const char*);
    LogStream& operator<<(const std::string&);
    
};


//生成一条log字符串
class Logger {
    public:
    enum LogLevel {
        TRACE = 0,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL
    };

  
    Logger(const char* sourseFileName, int line, bool isAbort);
    Logger(const char* sourseFileName, int line, LogLevel level);
    Logger(const char* sourseFileName, int line, LogLevel level, const char* func);
    ~Logger();
    LogStream& stream() { return _stream; };
    static LogLevel logLevel();
    using OutPutFuc = std::function<void(const char*, int)>;
    using FlushFunc = std::function<void()>;
    static void setOutPut(OutPutFuc);
    static void setFlush(FlushFunc);
    static void setLogLevel(LogLevel);

  private:
    void formatTime();
    //提供统一的组装log字符串的方法
    void impl(LogLevel level, int savedErrono, const char* fileName);
    LogStream _stream;
    std::string _sourseFile;
    LogLevel _level;
    int _line;
    std::string _func;

};
extern Logger::LogLevel g_logLevel;


#define LOG_TRACE if (Logger::logLevel() <= Logger::TRACE) \
  Logger(__FILE__, __LINE__, Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (Logger::logLevel() <= Logger::DEBUG) \
  Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()
#define LOG_INFO if (Logger::logLevel() <= Logger::INFO) \
  Logger(__FILE__, __LINE__, Logger::INFO).stream()
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::ERROR).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::FATAL).stream()
#define LOG_SYSERR Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL Logger(__FILE__, __LINE__, true).stream()
