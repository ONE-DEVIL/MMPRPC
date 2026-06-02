#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"

int main(int argc, char **argv)
{
    // 调用框架初始化操作(值初始化1次)
    MprpcApplication::Init(argc, argv);
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
    //rpc方法的请求参数
    fixbug::LoginRequest request_l;
    request_l.set_name("zhang san");
    request_l.set_pwd("123456");
    // rpc方法的响应
    fixbug::LoginResponse response_l;
    //
    MprpcController controller_l;
    // 发起rpc方法的调用，等待返回结果
    stub.Login(&controller_l, &request_l, &response_l, nullptr); // RpcChannel->CallMethod集中来做所有rpc方法调用的参数序列化和网络发送
    if (controller_l.Failed())
    {
        std::cout << "rpc Login failed! error: " << controller_l.ErrorText() << std::endl;
        return 0;
    }
    else
    {
        if (response_l.result().errcode() == 0)
        {
            std::cout << "rpc Login response success:" << response_l.success() << std::endl;
        }
        else
        {
            std::cout << "rpc Login response error: " << response_l.result().errmsg() << std::endl;
        }
    }
    fixbug::RegisterRequest request_r;
    request_r.set_id(100);
    request_r.set_name("zhang san");
    request_r.set_pwd("123456");
    fixbug::RegisterResponse response_r;
    MprpcController controller_r;
    stub.Register(&controller_r, &request_r, &response_r, nullptr);
    if (controller_r.Failed())
    {
        std::cout << "rpc Register failed! error: " << controller_r.ErrorText() << std::endl;
        return 0;
    }
    else
    {
        if (response_r.result().errcode() == 0)
        {
            std::cout << "rpc Register response success:" << response_r.success() << std::endl;
        }
        else
        {
            std::cout << "rpc Register response error: " << response_r.result().errmsg() << std::endl;
        }
    }

    return 0;
}