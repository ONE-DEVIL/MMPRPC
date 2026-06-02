#pragma once
#include <unordered_map>
#include <string>
// rpcserver_ip=? rpcserver_port=? zookeeper_ip=? zookeeper_port=?
class MprpcConfig
{
public:
    // 负责加载解析配置文件
    void LoadConfigFile(const char *config_file);
    // 根据键获取值，查询配置项信息
    std::string load(const std::string &key);

private:
    std::unordered_map<std::string, std::string> m_configMap;
    // 去掉字符串前后的空格
    void Trim(std::string &str);
};