#include <iostream>
#include <string>
#include <regex>

int main()
{
    std::string req = "GET /liren/login?user=xiaoming&pass=123123 HTTP/1.1\r\n";
    
    std::smatch base_match; // 结果集

    // (GET|POST|HEAD|PUT|DELETE)   表示匹配并提取其中任意一个字符串
    // [^?]*                        [^?] 匹配非问号字符，后边的*表示 0次或多次
    // \\?(.*)                      \\? 表示原始的 ? 字符，(.*)表示提取 ? 之后的任意字符 0 次或多次，直到遇到空格
    // (?:\\?(.*))?                 表示匹配了上一条内容 0 次或 1 次，并且不获取该内容
    // HTTP/1\\.[01]                表示匹配以 HTTP/1. 开始，后边有个 0 或 1 的字符串
    // (?:\n|\r\n)?                 (?: ...) 表示匹配某个格式字符串，但是不提取，最后的 ? 表示的是匹配前边的表达式 0 次或 1 次
    std::regex base_regex("(GET|POST|HEAD|PUT|DELETE) ([^?]*)(?:\\?(.*))? (HTTP/1\\.[01])(?:\n|\r\n)?");

    // 进行匹配
    bool ret = std::regex_match(req, base_match, base_regex);
    if(ret == false)
        return -1;
    
    // 输出匹配内容
    for(int i = 0; i < base_match.size(); ++i)
    {
        std::cout << i << " : ";
        std::cout << base_match[i] << std::endl;
    }

    return 0;
}