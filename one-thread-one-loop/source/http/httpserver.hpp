#include "../server.hpp"
#include <fstream>
#include <ctype.h>

class Util
{
public:
    // 字符串分割函数，将src字符串按照sep字符串进行分割，得到的各个子串放到arr中，最终返回子串的数量
    static size_t split(const std::string& src, const std::string& sep, std::vector<std::string>* arr)
    {
        // src："There are two needles in this haystack with needles."
        // sep："needle"
        // 则第一个位置为14
        size_t cur = src.find(sep);
        size_t pre = 0;
        while(cur != std::string::npos)
        {
            if(cur - pre > 0)
                arr->push_back(src.substr(pre, cur - pre)); // 要过滤掉空串的情况
            pre = cur + sep.size();
            cur = src.find(sep, cur + sep.size());
        }   
        if(src.size() - pre > 0)
            arr->push_back(src.substr(pre)); // 别忘了最后一个子串（也要判断是否为空串）
        
        return arr->size();
    }

    // 从文件读取内容，将读取的内容放到一个buffer中
    static bool read_file(const std::string& filename, std::string* buffer)
    {
        // 1. 创建二进制输入文件流
        std::ifstream ifs(filename, std::ios::binary);
        if(ifs.is_open() == false)
        {
            ELOG("open %s file failed!", filename.c_str());
            return false;
        }

        // 2. 偏移到末尾，然后通过此时的偏移量来获取文件的大小，最后别忘了重新偏移到开头
        ifs.seekg(0, ifs.end); 
        size_t file_size = ifs.tellg();
        ifs.seekg(0, ifs.beg);

        // 3. 根据文件大小写到buffer中
        buffer->resize(file_size);
        ifs.read(&(*buffer)[0], file_size); // 因为c_str()返回的是常量字符串，和参数不匹配，所以这里只能用取地址的方式传入buffer
        if(ifs.good() == false)
        {
            ELOG("read %s file failed!", filename.c_str());
            ifs.close();
            return false;
        }

        // 4. 最后关闭文件流
        ifs.close();
        return true;
    }

    // 向文件写入内容，要写的内容存放在buffer中
    static bool write_file(const std::string& filename, const std::string& buffer)
    {
        // 1. 创建二进制输出文件流
        std::ofstream ofs(filename, std::ios::binary);
        if(ofs.is_open() == false)
        {
            ELOG("open %s file failed!", filename.c_str());
            return false;
        }

        // 2. 将buffer中的数据写入文件中
        ofs.write(buffer.c_str(), buffer.size());
        if(ofs.good() == false)
        {
            ELOG("write %s file failed!", filename.c_str());
            ofs.close();
            return false;
        }

        // 3. 最后关闭文件流
        ofs.close();
        return true;
    }

    // URL编码，避免URL中资源路径与查询字符串中的特殊字符与HTTP请求中特殊字符产生歧义
    // 编码格式：将特殊字符的ascii值，转换为两个16进制字符以及一个前缀%   比如C++ -> C%2B%2B
    //   不编码的特殊字符： RFC3986文档规定 . - _ ~ 字母，数字属于绝对不编码字符
    // RFC3986文档规定，编码格式 %HH 
    // W3C标准中规定，查询字符串中的空格，需要编码为+，而解码则是+转空格
    static std::string url_encode(const std::string& url, bool convert_space_to_plus)
    {
        std::string ret;
        for(int i = 0; i < url.size(); ++i)
        {
            // 数字、字母以及不编码的字符直接尾插即可
            if(url[i] == '.' || url[i] == '-' || url[i] == '_' || url[i] == '~' || isalnum(url[i])) 
                ret += url[i];
            else if(url[i] == ' ' && convert_space_to_plus == true) // 如果为空格并且要求转化为+而不是编码的话，则直接尾插+即可
                ret += '+';
            else
            {
                // 剩下的就是要编码的符号了！
                // 下面直接使用 snprintf() 函数，格式化然后存放到tmp中，最后再尾插到ret即可
                char tmp[4] = { 0 };
                snprintf(tmp, 4, "%%%02X", url[i]); // %%表示输出真实的%，%02X表示输出占2位，不够位的话补充前置0的大写十六进制数
                ret += tmp;
            }
        }
        return ret;
    }

    // URL解码
    static std::string url_decode(const std::string& url, bool convert_plus_to_space)
    {   
        // 即遇到了%号则将紧随其后的2个字符转换为数字，第一个数字左移4位（也就是乘以16）然后加上第二个数字，比如%2b -> 2<<4+11 = 43
        std::string ret;
        for(int i = 0; i < url.size(); ++i)
        {
            // 对于+号的特殊处理
            if(url[i] == '+' && convert_plus_to_space == true)
            {
                ret += ' ';
                continue;
            }

            if(url[i] == '%')
            {
                int n1 = hex_to_dec(url[i + 1]);
                int n2 = hex_to_dec(url[i + 2]);
                ret += (char)((n1 << 4) + n2);
                i += 2;
                continue;
            }

            // 剩下的就是不需要解码的
            ret += url[i];
        }
        return ret;
    }

    // 十六进制转十进制的功能函数
    static int hex_to_dec(char c)
    {
        if(c >= '0' && c <= '9')
            return c - '0';
        else if(c >= 'a' && c <= 'z')
            return c - 'a' + 10;
        else if(c >= 'A' && c <= 'Z')
            return c - 'A' + 10;
        return -1;
    }

    // 通过HTTP状态码获取描述信息
    static std::string get_information_from_status(int status);

    // 根据文件后缀名获取mime（即想要请求的文件格式）
    static std::string get_mime_from_suffix(const std::string& filename);

    // 判断一个文件是否是目录
    static bool is_directory(const std::string& filename);

    // 判断一个文件是否是一个普通文件
    static bool is_regular_file(const std::string& filename);

    // 判断一个HTTP资源路径是否有效
    //      /index.html  --- 前边的/叫做相对根目录，映射的是某个服务器上的子目录
    //      想表达的意思就是，客户端只能请求相对根目录中的资源，其他地方的资源都不予理会
    //      而/../login --- 这个路径中的..会让路径的查找跑到相对根目录之外，这是不合理的，不安全的
    static bool is_path_valid(const std::string& path);
};