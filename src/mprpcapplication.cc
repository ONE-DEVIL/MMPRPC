#include "mprpcapplication.h"
#include "unistd.h"
#include <string>
#include <iostream>
MprpcConfig MprpcApplication::m_config;
int MprpcApplication::initCount = 0;
void ShowArgsHelp()
{
    std::cout << "format: command -i config.conf" << std::endl;
}
void MprpcApplication::Init(int argc, char **argv)
{
    // 解析命令行参数，加载配置文件等等
    if (argc < 2)
    {
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }
    int c = 0;
    std::string config_file;
    while ((c = getopt(argc, argv, "i:")) != -1)
    {
        switch (c)
        {
        case 'i':
            // 加载配置文件
            config_file = optarg;
            break;
        case '?':
            std::cout << "invalid command!" << std::endl;
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        case ':':
            std::cout << "please input config file!" << std::endl;
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }
    // 加载配置文件 rpcserver_ip=? rpcserver_port=? zookeeper_ip=? zookeeper_port=?
    m_config.LoadConfigFile(config_file.c_str());
    // std::cout<<"rpcserverip: "<<m_config.load("rpcserverip")<<std::endl;
    // std::cout<<"rpcserverport: "<<m_config.load("rpcserverport")<<std::endl;
    // std::cout<<"zookeeperip: "<<m_config.load("zookeeperip")<<std::endl;
    // std::cout<<"zookeeperport: "<<m_config.load("zookeeperport")<<std::endl;
    initCount++;
}
MprpcApplication &MprpcApplication::GetInstance()
{
    static MprpcApplication app;
    return app;
}
// 获取初始化次数
int MprpcApplication::GetInitCount()
{
    return initCount;
}
// 获取配置对象
MprpcConfig &MprpcApplication::GetConfig()
{
    return m_config;
}