#pragma once

#include <google/protobuf/service.h>
#include <string>

class MprpcController : public ::google::protobuf::RpcController
{
public:
    MprpcController();
    // 重置当前rpc调用的状态，使其可以被重用
    void Reset() override;
    // 判断当前rpc调用是否失败，返回true表示失败
    bool Failed() const override;
    // 返回一个描述当前rpc调用失败原因的字符串
    std::string ErrorText() const override;
    // 失败状态设置为true，并把reason作为失败原因
    void SetFailed(const std::string &reason) override;
    // 目前为实现具体的功能
    void StartCancel() override;
    bool IsCanceled() const override;
    void NotifyOnCancel(::google::protobuf::Closure *callback) override;

private:
    bool m_failed;         // 记录rpc调用是否失败
    std::string m_errText; // 记录rpc调用失败的错误信息
};
