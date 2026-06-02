#include <iostream>
#include <string>
#include "mprpcapplication.h"
#include "mprpcprovider.h"
#include "friend.pb.h"
#include "mprpccontroller.h"
class FriendService : public fixbug::FriendServiceRpc
{ // 使用在RPC发布端
    std::vector<std::string> GetFriendList(uint32_t user_id)
    {
        std::cout << "执行GetFriendList逻辑!" << std::endl;
        std::cout << "用户id:" << user_id << std::endl;
        std::vector<std::string> friend_list;
        friend_list.push_back("friend1");
        friend_list.push_back("friend2");
        friend_list.push_back("friend3");
        return friend_list;
    }
    // 实现基类FriendServiceRpc的虚函数，下面这些方法都是框架直接调用的
    void GetFriendList(::google::protobuf::RpcController *controller,
                    const ::fixbug::GetFriendListRequest *request,
                    ::fixbug::GetFriendListResponse *response,
                    ::google::protobuf::Closure *done)
    {
        // 取出数据
        uint32_t user_id = request->userid();
        // 执行本地的GetFriendList函数
        std::vector<std::string> friend_list = GetFriendList(user_id);
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        // 把返回结果写入GetFriendListResponse
        for (std::string &friend_name : friend_list)
        {
            std::string *p = response->add_friends();
            *p = friend_name;
        }
        // 调用Closure的Run方法，将响应返回给客户端
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
    provider.NotifyService(new FriendService());
    // 启动RPC服务提供者，Run以后，进程进入阻塞状态，等待远程RPC调用请求
    provider.Run();
    return 0;
}