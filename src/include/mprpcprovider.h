#pragma once
#include "google/protobuf/service.h"
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <google/protobuf/descriptor.h>
#include <string>
#include <unordered_map>
#include"logger.h"
// 框架专门提供的发布rpc服务的网络对象类
class RpcProvider
{
public:
    // 把服务发布出去，让rpc服务可以被调用
    void NotifyService(google::protobuf::Service *service);
    // 启动rpc节点，开始提供rpc远程调用服务
    void Run();

private:
    // service服务类型信息
    struct ServiceInfo
    {
        google::protobuf::Service *m_service;                                                    // 保存服务对象
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor *> m_methodMap; // 保存服务方法的描述信息
    };
    // 存储注册成功的服务对象和方法信息 key:service_name value:ServiceInfo
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;
    // 事件循环对象
    muduo::net::EventLoop m_eventLoop;
    // 新的socket连接回调函数
    void OnConnection(const muduo::net::TcpConnectionPtr &conn);
    // 已建立连接的用户发来rpc调用请求时的回调函数
    void OnMessage(const muduo::net::TcpConnectionPtr &, muduo::net::Buffer *, muduo::Timestamp);
    // Clouser的回调操作,用于序列化rpc的响应和网络发送
    void sendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response);
};