#pragma once
#include "mprpcconfig.h"
#include "mprpccontroller.h"
#include "mprpcchannel.h"
// mprpc框架基础类
class MprpcApplication
{
public:
    // 初始化mprpc框架，加载配置文件等等
    static void Init(int argc, char **argv);
    // 获取mprpc框架单例对象
    static MprpcApplication &GetInstance();
    // 获取初始化次数
    static int GetInitCount();
    // 获取配置对象
    MprpcConfig &GetConfig();

private:
    // 初始化次数
    static int initCount;
    // 配置类对象
    static MprpcConfig m_config;
    // 单例模式，禁止外部构造和复制
    MprpcApplication() {}
    MprpcApplication(const MprpcApplication &) = delete;
    MprpcApplication &operator=(const MprpcApplication &) = delete;
};