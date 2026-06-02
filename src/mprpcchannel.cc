#include "mprpcchannel.h"
#include <string>
#include"rpcheader.pb.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include<netinet/in.h>
#include<memory>
#include "mprpcapplication.h"
#include"mprpccontroller.h"
#include"logger.h"
#include"zookeeperutil.h"
/*
haeder_size+header_str+args_str     header_str = service_name method_name args_size
*/
// 所有通过stub调用的rpc方法都走这里，统一做rpc方法调用的数据序列化和网络发送
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                              google::protobuf::RpcController *controller,
                              const google::protobuf::Message *request,
                              google::protobuf::Message *response,
                              google::protobuf::Closure *done)
{
    const google::protobuf::ServiceDescriptor *sd = method->service();
    std::string service_name = sd->name();
    std::string method_name = method->name();
    // 获取参数的序列化字符串长度
    std::string args_str;
    u_int32_t arg_size = 0;
    if (request->SerializeToString(&args_str))
    {
        arg_size = args_str.size();
    }
    else
    {
        //std::cout << "Serialize request failed!" << std::endl;
        controller->SetFailed("Serialize request failed!");
        return;
    }
    // 定义rpc方法调用的请求header
    mprpc::RpcHeader rpc_header;
    rpc_header.set_service_name(service_name);
    rpc_header.set_method_name(method_name);
    rpc_header.set_args_size(arg_size);
    // 序列化rpc方法调用的请求header
    std::string rpc_header_str;
    uint32_t header_size = 0;
    if (rpc_header.SerializeToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();
    }
    else
    {
        //std::cout << "Serialize rpc header failed!" << std::endl;
        controller->SetFailed("Serialize rpc header failed!");
        return;
    }
    // 组织rpc方法调用的请求字符串
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char *)&header_size, 4));
    send_rpc_str += rpc_header_str;
    send_rpc_str += args_str;
    // 打印调试信息
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    //std::cout << "send_rpc_str: " << send_rpc_str << std::endl;
    // 通过网络把rpc方法调用请求发送给rpc服务提供者通过tcp编程
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    //改为自定义智能指针，在删除器里释放
    // std::shared_ptr<int> clientfd(new int(socket(AF_INET, SOCK_STREAM, 0)), [](int *p) {
    //     if (*p != -1)
    //     {
    //         close(*p);
    //     }
    //     delete p;
    // });
    if (-1 == clientfd)
    {
        //std::cout << "create socket error! errno:" << errno << std::endl;
        char errText[512] = {0};
        sprintf(errText, "create socket error! errno:%d", errno);
        controller->SetFailed(errText);
        return;
    }
    // 填写client需要连接的server信息ip+port
    // std::string ip = MprpcApplication::GetInstance().GetConfig().load("rpcserverip");
    // uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().load("rpcserverport").c_str());
    //改为从zk上发现服务
    ZkClient zkCli;
    zkCli.Start();
    std::string service_path = "/" + service_name;
    std::string method_path = service_path + "/" + method_name;
    std::string host_data = zkCli.GetData(method_path.c_str());
    if(host_data==""){
        controller->SetFailed(method_path + " is not exist!");
        return;
    }
    int idx = host_data.find(":");
    if(idx == -1){
        controller->SetFailed(method_path + " address is invalid!");
        return;
    }
    std::string ip = host_data.substr(0, idx);
    uint16_t port = atoi(host_data.substr(idx+1, host_data.size()-idx-1).c_str());
    //创建socket连接服务器
    struct sockaddr_in server_addr;
    //初始化server_addr结构体变量
    memset(&server_addr, 0, sizeof(sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    // 连接rpc节点
    if (-1 == connect(clientfd, (struct sockaddr *)&server_addr, sizeof(server_addr)))
    {
        //std::cout << "connect error! errno:" << errno << std::endl;
        char errText[512] = {0};
        sprintf(errText, "connect error! errno:%d", errno);
        controller->SetFailed(errText);
        close(clientfd);
        return;
    }
    // 发送rpc请求
    if (-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
    {
        //std::cout << "send error! errno:" << errno << std::endl;
        char errText[512] = {0};
        sprintf(errText, "send error! errno:%d", errno);
        controller->SetFailed(errText);
        close(clientfd);
        return;
    }
    // 接收rpc请求的响应
    char recv_buf[1024] = {0};
    int recv_size = 0;
    if (-1 == (recv_size = recv(clientfd, recv_buf, 1024, 0)))
    {
        //std::cout << "recv error! errno:" << errno << std::endl;
        char errText[512] = {0};
        sprintf(errText, "recv error! errno:%d", errno);
        controller->SetFailed(errText);
        close(clientfd);
        return;
    }
    // 把rpc响应反序列化到response对象里
    //std::string response_str(recv_buf, 0, recv_size);//bug问题，recv_buf中遇到'\0'就认为字符串结束了，导致response_str不完整
    if (!response->ParseFromArray(recv_buf, recv_size))
    {
        // 解析响应失败
        //std::cout << "parse response error! response_str:" << recv_buf << std::endl;
        char errText[512] = {0};
        sprintf(errText, "parse response error! response_str:%d", errno);
        controller->SetFailed(errText);
        close(clientfd);
        return;
    }
    // 关闭socket连接
    close(clientfd);
}