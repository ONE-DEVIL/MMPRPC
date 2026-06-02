#include "zookeeperutil.h"
#include "mprpcapplication.h"
#include"logger.h"
#include <semaphore.h>
#include <iostream>

// 全局的watcher观察器 zkserver给zkclient的通知
void global_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT)
    { // 回调的消息类型是和会话相关的消息类型
        if (state == ZOO_CONNECTED_STATE)
        {                                              // zkclient连接zkserver成功了
            sem_t *sem = (sem_t *)zoo_get_context(zh); // 从zkclient的上下文中获取信号量的地址
            sem_post(sem);                             // 发送信号，通知调用线程zkclient连接zkserver成功了
        }
    }
}

ZkClient::ZkClient() : m_zhandle(nullptr)
{
}
ZkClient::~ZkClient()
{
    if (m_zhandle != nullptr)
    {
        zookeeper_close(m_zhandle);
    }
}
// 连接zkserver
void ZkClient::Start()
{
    std::string host = MprpcApplication::GetInstance().GetConfig().load("zookeeperip");
    std::string port = MprpcApplication::GetInstance().GetConfig().load("zookeeperport");
    std::string connstr = host + ":" + port;
    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    /*
    zookeepr_mt是zookeeper的多线程版本
    zookeeper的API客户端程序提供了三个线程
    API调用线程
    网络I/O线程 pthred_create poll
    watcher回调线程
    */
    if (m_zhandle == nullptr)
    {
        //std::cout << "zookeeper_init error!" << std::endl;
        LOG_ERROR("zookeeper_init error!");
        exit(EXIT_FAILURE);
    }
    // 创建信号量，等待zkclient连接zkserver成功后，发送信号通知调用线程继续执行
    sem_t sem;
    // 信号量初始化为0
    sem_init(&sem, 0, 0);
    // 把信号量的地址设置到zkclient的上下文中，当连接成功后，zkclient会调用watcher回调函数，通知调用线程连接成功了
    zoo_set_context(m_zhandle, &sem);
    // 连接状态变化的回调函数，当连接成功了，发送信号通知调用线程继续执行
    sem_wait(&sem);
    //std::cout << "zookeeper_init success!" << std::endl;
    LOG_INFO("zookeeper_init success!");
}
// 在zkserver上根据指定path创建znode节点
void ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
    char path_buffer[128] = {0};
    int bufflen = sizeof(path_buffer);
    int flag;   
    // 先判断节点是否存在
    flag = zoo_exists(m_zhandle, path, 0, nullptr);
    if (ZNONODE == flag)
    {
        flag = zoo_create(m_zhandle, path, data, datalen, &ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufflen);
        if (flag == ZOK)
        {
            std::cout << "zookeeper create node success! path:" << path << std::endl;
            LOG_INFO("zookeeper create node success! path: %s", path);
        }
        else
        {
            std::cout << "zookeeper create node error! path:" << path << " error code:" << flag << std::endl;
            LOG_ERROR("zookeeper create node error! path: %s error code: %d", path, flag);
            exit(EXIT_FAILURE);
        }
    }
}
// 根据参数指定的znode节点path获取zookeeper节点的值
std::string ZkClient::GetData(const char *path)
{
    char data_buffer[128] = {0};
    int bufflen = sizeof(data_buffer);
    int flag = zoo_get(m_zhandle, path, 0, data_buffer, &bufflen, nullptr);
    if (flag != ZOK)
    {
        //std::cout << "zookeeper get node error! path:" << path << " error code:" << flag << std::endl;
        LOG_ERROR("zookeeper get node error! path: %s error code: %d", path, flag);
        return "";
    }
    else
    {
        return data_buffer;
    }
}