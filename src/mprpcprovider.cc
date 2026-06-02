#include "mprpcprovider.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include"zookeeperutil.h"

/*
service_name =>server描述  =>service* 记录服务对象
method_name => method方法描述
json 文本存储（键值对）   protobuf二进制存储(值)
*/
// 把服务发布出去，让rpc服务可以被调用
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;
    // 获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pServiceDesc = service->GetDescriptor();
    // 获取服务的名字
    std::string service_name = pServiceDesc->name();
    //std::cout << "service_name: " << service_name << std::endl;
    LOG_INFO("service_name: %s", service_name.c_str());
    // 获取服务对象的方法数量
    int methodCnt = pServiceDesc->method_count();
    for (int i = 0; i < methodCnt; ++i)
    {
        // 获取了服务对象的第i个方法的描述信息(抽象描述)
        const google::protobuf::MethodDescriptor *pMethodDesc = pServiceDesc->method(i);
        std::string method_name = pMethodDesc->name();
        //std::cout << "method_name: " << method_name << std::endl;
        LOG_INFO("method_name: %s", method_name.c_str());
        // 将方法名字和方法描述信息放入到服务信息的map中
        service_info.m_methodMap.insert({method_name, pMethodDesc});
    }
    service_info.m_service = service;
    m_serviceMap.insert({service_name, service_info});
}
// 启动rpc节点，开始提供rpc远程调用服务
void RpcProvider::Run()
{
    std::string ip = MprpcApplication::GetInstance().GetConfig().load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().load("rpcserverport").c_str());
    muduo::net::InetAddress addr(ip, port);

    // 创建TcpServer对象
    muduo::net::TcpServer server(&m_eventLoop, addr, "RpcProvider");
    // 绑定连接回调和消息读写回调方法， 分离了网络代码和业务代码
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    // 设置muduo库的线程数量
    server.setThreadNum(4);
    //把当前rpc节点上要发布的服务全部注册到zk上，让rpc client将可以从zk上发现服务
    //timeout 30s zkclient 网络IO线程 1/3*timeout发送ping消息给zkserver，保持心跳，防止连接断开
    ZkClient zkCli;
    zkCli.Start();
    // service_name => /service_name为永久节点，method_name => /service_name/method_name为临时节点，存储rpc方法的调用地址
    for(auto &sp : m_serviceMap){
        std::string service_path = "/"+sp.first;
        zkCli.Create(service_path.c_str(), nullptr, 0);
        for(auto &mp : sp.second.m_methodMap){
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            //ZOO_EPHEMERAL临时性节点
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }
    std::cout << "RpcProvider start service at " << ip << ":" << port << std::endl;
    LOG_INFO("RpcProvider start service at %s:%d", ip.c_str(), port);
    // 启动服务器
    server.start();
    // 进入事件循环，等待客户端的连接和rpc调用请求
    m_eventLoop.loop();
}
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        // 和rpc客户端的连接断开了
        conn->shutdown();
    }
}
/*
在框架内部 RpcProvider和RpcConsumer协商好之间通信用的protobuf数据类型
service_name method_name args   定义相应的proto和message类型，进行数据头的序列化和反序列化
                                       header_str=service_name+method_name+args_size
header_size+header_str+args_str
std::string insert ,copy
*/
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buffer, muduo::Timestamp time)
{
    // 网络上接收的远程rpc调用请求的字符流 login argv
    std::string recv_buf = buffer->retrieveAllAsString();
    // 从字符流中读取前4个字节的header_size
    uint32_t header_size = 0;
    recv_buf.copy((char *)&header_size, 4, 0);
    // 根据header_size读取数据头的字符流，进行反序列化得到服务名称和方法名称
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size = 0;
    if (rpcHeader.ParseFromString(rpc_header_str))
    {
        // 数据头反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else
    {
        // 数据头反序列化失败，记录日志
        //std::cout << "rpc_header_str: " << rpc_header_str << " parse error!" << std::endl;
        LOG_ERROR("rpc_header_str: %s parse error!", rpc_header_str.c_str());
        return;
    }
    // 获取rpc参数的字符流数据
    std::string args_str = recv_buf.substr(4 + header_size, args_size);
    // 打印调试信息
    // std::cout << "====================================================" << std::endl;
    // std::cout << "header_size: " << header_size << std::endl;
    // std::cout << "service_name: " << service_name << std::endl;
    // std::cout << "method_name: " << method_name << std::endl;
    // std::cout << "args_size: " << args_size << std::endl;
    // std::cout << "args_str: " << args_str << std::endl;
    // std::cout << "====================================================" << std::endl;
    LOG_INFO("====================================================");
    LOG_INFO("header_size: %d", header_size);
    LOG_INFO("service_name: %s", service_name.c_str());
    LOG_INFO("method_name: %s", method_name.c_str());
    LOG_INFO("args_size: %d", args_size);
    LOG_INFO("args_str: %s", args_str.c_str());
    LOG_INFO("====================================================");
    // 在本地注册表中查找服务对象和方法对象
    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end())
    {
        //std::cout << "service: " << service_name << " is not exist!" << std::endl;
        LOG_ERROR("service: %s is not exist!", service_name.c_str());
        return;
    }
    auto method_it = it->second.m_methodMap.find(method_name);
    if (method_it == it->second.m_methodMap.end())
    {
        //std::cout << "method: " << method_name << " is not exist!" << std::endl;
        LOG_ERROR("method: %s is not exist!", method_name.c_str());
        return;
    }
    // 获取服务对象和方法对象
    google::protobuf::Service *service = it->second.m_service;            // 获取服务对象
    const google::protobuf::MethodDescriptor *method = method_it->second; // 获取方法对象
    // 生成rpc方法调用的请求request和响应response参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str))
    {
        //std::cout << "request->ParseFromString(args_str) failed!" << args_str << std::endl;
        LOG_ERROR("request->ParseFromString(args_str) failed!");
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();
    // 给下面method方法的调用，绑定一个Clouser的回调函数
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider, const muduo::net::TcpConnectionPtr &, google::protobuf::Message *>(this, &RpcProvider::sendRpcResponse, conn, response);
    // 在框架上根据服务对象和方法对象，调用google提供的CallMethod方法进行rpc方法的调用，获取响应结果
    //相当于UserService.Login(controller, request, response, done) 由框架调用，业务只需要实现Login函数逻辑即可
    service->CallMethod(method, nullptr, request, response, done);
}
// Clouser的回调操作,用于序列化rpc的响应和网络发送
void RpcProvider::sendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
{
    std::string response_str;
    if (response->SerializeToString(&response_str))
    {                             // 序列化成功
        conn->send(response_str); // 通过网络把rpc方法调用的结果发送给rpc的调用方
        conn->shutdown();         // 模拟http短链接的方式，由rpcprovider断开连接
    }
    else
    { // 序列化失败
        //std::cout << "response->SerializeToString failed!" << std::endl;
        LOG_ERROR("response->SerializeToString failed!");
    }
    conn->shutdown(); // 模拟http短链接的方式，由rpcprovider断开连接
}