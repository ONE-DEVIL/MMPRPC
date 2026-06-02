#pragma once
#include "lockqueue.h"

// Mprpc提供的日志系统

enum LogLevel
{
    INFO,
    ERROR

};
class Logger
{
public:
    //析构
    ~Logger();
    // 获取日志单例对象
    static Logger &GetInstance();
    // 设置日志级别
    void SetLogLevel(int level);
    // 写日志
    void Log(std::string msg);

private:
    int m_logLevel;                  // 日志级别
    LockQueue<std::string> m_lckQue; // 日志缓冲区
    Logger();
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;
};
#define LOG_INFO(logmsgformat,...) \
    do{\
        Logger &logger = Logger::GetInstance();\
        logger.SetLogLevel(INFO);\
        char logmsg[1024] = {0};\
        snprintf(logmsg, 1024, logmsgformat, ##__VA_ARGS__);\
        logger.Log(logmsg);\
    }while(0)

#define LOG_ERROR(logmsgformat,...) \
    do{\
        Logger &logger = Logger::GetInstance();\
        logger.SetLogLevel(ERROR);\
        char logmsg[1024] = {0};\
        snprintf(logmsg, 1024, logmsgformat, ##__VA_ARGS__);\
        logger.Log(logmsg);\
    }while(0)