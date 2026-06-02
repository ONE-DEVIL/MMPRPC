#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
// 异步写日志的日志队列

template <typename T>
class LockQueue
{
public:
    //多个工作线程都会写日志
    void Push(const T &data){
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(data);
        m_cond.notify_one();
    }
    bool Pop(T &data){
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty())
        {
            // 如果收到退出信号且队列为空，返回false告知线程结束
            if (m_exit) 
            {
                return false;
            }
            // 日志队列为空，线程进入wait状态
            m_cond.wait(lock);
        }
        data = m_queue.front();
        m_queue.pop();
        return true;
    }
    // 新增：主动唤醒并关闭队列
    void Stop()
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_exit = true;
        }
        m_cond.notify_all(); // 唤醒所有正在wait的线程
    }
private:
    std::queue<T> m_queue;          // 队列
    std::mutex m_mutex;             // 互斥锁
    std::condition_variable m_cond; // 条件变量
    bool m_exit = false; // 退出标志 ,若没有这个，在pop是会陷入死锁，导致程序无法正常退出，应为pop函数中的wait会一直等待，无法被唤醒退出循环
};