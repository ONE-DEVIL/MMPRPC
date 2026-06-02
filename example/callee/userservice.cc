#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "mprpcprovider.h"
/*
userService原本是一个本地服务，提供了两个进程内的本地方法Login和GetFriendList
*/
class UserService:public fixbug::UserServiceRpc
{
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "执行登录逻辑！" << std::endl;
        std::cout << "用户名:" << name << " 密码：" << pwd << std::endl;
        return true;
    }
    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "执行注册逻辑！" << std::endl;
        std::cout << "用户id:" << id << " 用户名:" << name << " 密码：" << pwd << std::endl;
        return true;
    }
    /*
    实现基类UserServiceRpc的虚函数，下面这些方法都是框架直接调用的
    caller ==>Login(LoginRequest) ==>muduo ==> callee
    callee ==>Login(LoginRequest) ==> callee实现的Login(LoginRequest)
    */
    void Login(::google::protobuf::RpcController *controller,
            const ::fixbug::LoginRequest *request,
            ::fixbug::LoginResponse *response,
            ::google::protobuf::Closure *done)
    {
        // 框架给业务上报了请求参数LoginRequest，业务获取LoginRequest里面的参数，执行本地的Login函数
        std::string name = request->name();
        std::string pwd = request->pwd();
        bool login_result = Login(name, pwd); // 本地的Login函数
        // 把本地Login函数的返回值设置到LoginResponse里，框架会把LoginResponse发送给caller
        // 把响应写入，包括错误码、错误消息、返回值
        fixbug::RequestCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(login_result);
        // 执行回调操作，通知框架Login函数调用完成，框架就会把LoginResponse发送给caller
        done->Run();
    }
    void Register(::google::protobuf::RpcController *controller,
                const ::fixbug::RegisterRequest *request,
                ::fixbug::RegisterResponse *response,
                ::google::protobuf::Closure *done)
    {
        // 框架给业务上报了请求参数RegisterRequest，业务获取RegisterRequest里面的参数，执行本地的Register函数
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();
        bool register_result = Register(id, name, pwd); // 本地的Register函数
        // 把本地Register函数的返回值设置到RegisterResponse里，框架会把RegisterResponse发送给caller
        // 把响应写入，包括错误码、错误消息、返回值
        fixbug::RequestCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(register_result);
        // 执行回调操作，通知框架Register函数调用完成，框架就会把RegisterResponse发送给caller
        done->Run();
    }
};
int main(int argc, char **argv)
{
    LOG_INFO("first log message!");
    LOG_ERROR("%s:%s:%d", __FILE__, __FUNCTION__, __LINE__);
    // 调用框架初始化操作 provider -i config.conf
    if (MprpcApplication::GetInitCount() == 0)
    {
        MprpcApplication::Init(argc, argv);
    }
    // provider是一个RPC服务发布对象，负责把UserService对象发布到RPC节点上，供RPC客户端调用
    RpcProvider provider;
    provider.NotifyService(new UserService());
    // 启动RPC服务提供者，Run以后，进程进入阻塞状态，等待远程RPC调用请求
    provider.Run();
    return 0;
}