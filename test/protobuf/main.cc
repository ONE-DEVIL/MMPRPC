#include<iostream>
#include<string>
#include"test.pb.h"
using namespace fixbug;
int main(){
    // LoginResponse rsp;
    // RequestCode *rc=rsp.mutable_result();
    // rc->set_errcode(1);
    // rc->set_errmsg("登录失败，用户名或密码错误！");

    GetFriendListResponse grsp;
    RequestCode *rc = grsp.mutable_result();
    rc->set_errcode(1);
    rc->set_errmsg("获取好友列表失败，用户未登录！");
    User *user1 = grsp.add_friend_lists();
    user1->set_name("zhangsan");
    user1->set_age(20);
    user1->set_sex(User::MAN);
    User *user2 = grsp.add_friend_lists();
    user2->set_name("lisi");
    user2->set_age(25);
    user2->set_sex(User::WOMAN);
    std::cout<<grsp.friend_lists_size()<<std::endl;
    for(int i=0;i<grsp.friend_lists_size();++i){
        const User &user = grsp.friend_lists(i);
        std::cout<<"name:"<<user.name()<<std::endl;
        std::cout<<"age:"<<user.age()<<std::endl;
    }

    return 0;
}
// int main1(){
//     //封装了login请求的消息对象
//     LgoinRequest request;
//     request.set_name("zhangsan");
//     request.set_pwd("123456");
//     std::string send_str;
//     //将消息对象序列化成 char*
//     if(request.SerializeToString(&send_str)){
//         std::cout<<"序列化成功！"<<std::endl;
//         std::cout<<"序列化后的字符串为："<<send_str.c_str()<<std::endl;
//     }

//     //从send_str反序列化一个login请求对象
//     LgoinRequest request2;
//     if(request2.ParseFromString(send_str)){
//         std::cout<<"反序列化成功！"<<std::endl;
//         std::cout<<"反序列化后的name为："<<request2.name()<<std::endl;
//         std::cout<<"反序列化后的pwd为："<<request2.pwd()<<std::endl;
//     }
//     //封装login响应的消息对象
//     LoginResponse response;
//     response.set_errmsg("登录失败，用户名或密码错误！");
//     response.set_errcode(100);
//     response.set_success(false);
//     std::string send_str2;
//     //将消息对象序列化成 char*
//     if(response.SerializeToString(&send_str2)){
//         std::cout<<"序列化成功！"<<std::endl;
//         std::cout<<"序列化后的字符串为："<<send_str2.c_str()<<std::endl;
//     }
//     //从send_str2反序列化一个login响应对象
//     LoginResponse response2;
//     if(response2.ParseFromString(send_str2)){
//         std::cout<<"反序列化成功！"<<std::endl;
//         std::cout<<"反序列化后的errmsg为："<<response2.errmsg()<<std::endl;
//         std::cout<<"反序列化后的errcode为："<<response2.errcode()<<std::endl;
//         std::cout<<"反序列化后的success为："<<response2.success()<<std::endl;
//     }
//     return 0;

// }