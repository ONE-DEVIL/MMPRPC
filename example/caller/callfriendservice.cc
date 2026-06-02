#include <iostream>
#include "mprpcapplication.h"
#include "friend.pb.h"

int main(int argc, char **argv)
{
    // 调用框架初始化操作(值初始化1次)
    MprpcApplication::Init(argc, argv);
    // 演示远程调用发布的rpc方法GetFriendList
    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel());
    // rpc方法的请求参数
    fixbug::GetFriendListRequest request_g;
    request_g.set_userid(100);
    // rpc方法的响应
    fixbug::GetFriendListResponse response_g;
    // 发起rpc方法的调用，同步的rpc调用过程，等待返回结果
    MprpcController controller;
    stub.GetFriendList(&controller, &request_g, &response_g, nullptr);
    // 处理rpc方法的响应结果
    if (controller.Failed())
    {
        std::cout << "rpc GetFriendList failed! error: " << controller.ErrorText() << std::endl;
        return 0;
    }
    else
    {
        if (response_g.result().errcode() == 0)
        {
            std::cout << "rpc GetFriendList response success:" << std::endl;
            for (int i = 0; i < response_g.friends_size(); ++i)
            {
                //std::cout << "rpc GetFriendList response success!" << std::endl;
                std::cout << " friend" << i << ": " << response_g.friends(i) << std::endl;
            }
        }
        else
        {
            std::cout << "rpc GetFriendList response error: " << response_g.result().errmsg() << std::endl;
        }
    }

    return 0;
}