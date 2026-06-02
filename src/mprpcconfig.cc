#include "mprpcconfig.h"
#include <iostream>
#include <string>
// 负责加载解析配置文件
void MprpcConfig::LoadConfigFile(const char *config_file)
{
    FILE *pf = fopen(config_file, "r");
    if (pf == nullptr)
    {
        std::cout << "config file:" << config_file << " is not exist!" << std::endl;
        exit(EXIT_FAILURE);
    }
    // 1.注释 2.正确的配置项 = 3.开头的空格
    while (!feof(pf))
    {
        char buf[512] = {0};
        fgets(buf, 512, pf);
        // 去掉字符串开头的空格
        std::string read_buf(buf);
        Trim(read_buf);
        // 判断注释和空行
        if (read_buf[0] == '#' || read_buf.empty())
        {
            continue;
        }
        // 正确的配置项
        int idx = read_buf.find('=');
        if (idx == -1)
        {
            // 说明没有=，不合法的配置项
            continue;
        }
        std::string key = read_buf.substr(0, idx);
        Trim(key);
        int endidx = read_buf.find('\n', idx);
        std::string value = read_buf.substr(idx + 1, endidx - idx - 1);
        Trim(value);
        m_configMap[key] = value;
    }
}
// 根据键获取值，查询配置项信息
std::string MprpcConfig::load(const std::string &key)
{
    auto it = m_configMap.find(key);
    if (it != m_configMap.end())
    {
        return it->second;
    }
    return "";
}
// 去掉字符串前后的空格
void MprpcConfig::Trim(std::string &str)
{
    int idx = str.find_first_not_of(' ');
    if (idx != -1)
    {
        // 说明前面有空格
        str = str.substr(idx, str.size() - idx);
    }
    // 去掉字符串末尾的空格
    idx = str.find_last_not_of(' ');
    if (idx != -1)
    {
        // 说明后面有空格
        str = str.substr(0, idx + 1);
    }
}