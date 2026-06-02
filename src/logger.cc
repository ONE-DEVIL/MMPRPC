#include"logger.h"
#include<time.h>
#include<string>
#include<iostream>
#include<pthread.h>
//获取日志单例对象
Logger &Logger::GetInstance()
{
    static Logger logger;
    return logger;
}
//日志构造函数，启动专门的写日志线程
Logger::Logger(){
    //启动专门的写日志线程
    std::thread writeLogTask([&](){
        for(;;){
            //获取当天的日期，然后取日志信息，写入相应的文件中 a+
            time_t now = time(0);
            tm *nowtm = localtime(&now);
            char fileName[128] = {0};
            sprintf(fileName, "%d-%d-%d-log.txt", 1900 + nowtm->tm_year, 1 + nowtm->tm_mon, nowtm->tm_mday);
            FILE * pf = fopen(fileName, "a+");
            if(pf == nullptr){
                std::cout << "open log file error!" << std::endl;
                exit(EXIT_FAILURE);
            }
            //std::string log = m_lckQue.Pop();
            // 1. 获取数据，若Pop返回false则说明队列关闭且已空，直接退出循环
            std::string log;
            if (!m_lckQue.Pop(log)) 
            {
                break;
            }
            char time_buf[128] = {0};
            sprintf(time_buf, "%d:%d:%d", nowtm->tm_hour, nowtm->tm_min, nowtm->tm_sec);
            log.insert(0, time_buf);
            log.append("\n");
            fputs(log.c_str(), pf);
            fclose(pf);

        }
    });
    //设置分离线程
    writeLogTask.detach();
}
//设置日志级别
void Logger::SetLogLevel(int level)
{
    m_logLevel = level;
}
//写日志
void Logger::Log(std::string msg)
{
    if(m_logLevel == INFO){
        msg = "=>[info] " + msg;
    }
    else if(m_logLevel == ERROR){
        msg = "=>[error] " + msg;
    }
    m_lckQue.Push(msg); 
}
// 新增：析构函数。程序退出时销毁单例，触发此函数唤醒后台线程退出循环
Logger::~Logger()
{
    m_lckQue.Stop();
}