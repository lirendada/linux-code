#include "../server.hpp"
#include <fstream>
#include <ctype.h>

// 状态码对应信息的哈希表
std::unordered_map<int, std::string> status_msg = {
    {100,  "Continue"},
    {101,  "Switching Protocol"},
    {102,  "Processing"},
    {103,  "Early Hints"},
    {200,  "OK"},
    {201,  "Created"},
    {202,  "Accepted"},
    {203,  "Non-Authoritative Information"},
    {204,  "No Content"},
    {205,  "Reset Content"},
    {206,  "Partial Content"},
    {207,  "Multi-Status"},
    {208,  "Already Reported"},
    {226,  "IM Used"},
    {300,  "Multiple Choice"},
    {301,  "Moved Permanently"},
    {302,  "Found"},
    {303,  "See Other"},
    {304,  "Not Modified"},
    {305,  "Use Proxy"},
    {306,  "unused"},
    {307,  "Temporary Redirect"},
    {308,  "Permanent Redirect"},
    {400,  "Bad Request"},
    {401,  "Unauthorized"},
    {402,  "Payment Required"},
    {403,  "Forbidden"},
    {404,  "Not Found"},
    {405,  "Method Not Allowed"},
    {406,  "Not Acceptable"},
    {407,  "Proxy Authentication Required"},
    {408,  "Request Timeout"},
    {409,  "Conflict"},
    {410,  "Gone"},
    {411,  "Length Required"},
    {412,  "Precondition Failed"},
    {413,  "Payload Too Large"},
    {414,  "URI Too Long"},
    {415,  "Unsupported Media Type"},
    {416,  "Range Not Satisfiable"},
    {417,  "Expectation Failed"},
    {418,  "I'm a teapot"},
    {421,  "Misdirected Request"},
    {422,  "Unprocessable Entity"},
    {423,  "Locked"},
    {424,  "Failed Dependency"},
    {425,  "Too Early"},
    {426,  "Upgrade Required"},
    {428,  "Precondition Required"},
    {429,  "Too Many Requests"},
    {431,  "Request Header Fields Too Large"},
    {451,  "Unavailable For Legal Reasons"},
    {501,  "Not Implemented"},
    {502,  "Bad Gateway"},
    {503,  "Service Unavailable"},
    {504,  "Gateway Timeout"},
    {505,  "HTTP Version Not Supported"},
    {506,  "Variant Also Negotiates"},
    {507,  "Insufficient Storage"},
    {508,  "Loop Detected"},
    {510,  "Not Extended"},
    {511,  "Network Authentication Required"}
};

// 文件后缀对应资源格式的哈希表
std::unordered_map<std::string, std::string> mime_msg = {
    {".aac",        "audio/aac"},
    {".abw",        "application/x-abiword"},
    {".arc",        "application/x-freearc"},
    {".avi",        "video/x-msvideo"},
    {".azw",        "application/vnd.amazon.ebook"},
    {".bin",        "application/octet-stream"},
    {".bmp",        "image/bmp"},
    {".bz",         "application/x-bzip"},
    {".bz2",        "application/x-bzip2"},
    {".csh",        "application/x-csh"},
    {".css",        "text/css"},
    {".csv",        "text/csv"},
    {".doc",        "application/msword"},
    {".docx",       "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
    {".eot",        "application/vnd.ms-fontobject"},
    {".epub",       "application/epub+zip"},
    {".gif",        "image/gif"},
    {".htm",        "text/html"},
    {".html",       "text/html"},
    {".ico",        "image/vnd.microsoft.icon"},
    {".ics",        "text/calendar"},
    {".jar",        "application/java-archive"},
    {".jpeg",       "image/jpeg"},
    {".jpg",        "image/jpeg"},
    {".js",         "text/javascript"},
    {".json",       "application/json"},
    {".jsonld",     "application/ld+json"},
    {".mid",        "audio/midi"},
    {".midi",       "audio/x-midi"},
    {".mjs",        "text/javascript"},
    {".mp3",        "audio/mpeg"},
    {".mpeg",       "video/mpeg"},
    {".mpkg",       "application/vnd.apple.installer+xml"},
    {".odp",        "application/vnd.oasis.opendocument.presentation"},
    {".ods",        "application/vnd.oasis.opendocument.spreadsheet"},
    {".odt",        "application/vnd.oasis.opendocument.text"},
    {".oga",        "audio/ogg"},
    {".ogv",        "video/ogg"},
    {".ogx",        "application/ogg"},
    {".otf",        "font/otf"},
    {".png",        "image/png"},
    {".pdf",        "application/pdf"},
    {".ppt",        "application/vnd.ms-powerpoint"},
    {".pptx",       "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
    {".rar",        "application/x-rar-compressed"},
    {".rtf",        "application/rtf"},
    {".sh",         "application/x-sh"},
    {".svg",        "image/svg+xml"},
    {".swf",        "application/x-shockwave-flash"},
    {".tar",        "application/x-tar"},
    {".tif",        "image/tiff"},
    {".tiff",       "image/tiff"},
    {".ttf",        "font/ttf"},
    {".txt",        "text/plain"},
    {".vsd",        "application/vnd.visio"},
    {".wav",        "audio/wav"},
    {".weba",       "audio/webm"},
    {".webm",       "video/webm"},
    {".webp",       "image/webp"},
    {".woff",       "font/woff"},
    {".woff2",      "font/woff2"},
    {".xhtml",      "application/xhtml+xml"},
    {".xls",        "application/vnd.ms-excel"},
    {".xlsx",       "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
    {".xml",        "application/xml"},
    {".xul",        "application/vnd.mozilla.xul+xml"},
    {".zip",        "application/zip"},
    {".3gp",        "video/3gpp"},
    {".3g2",        "video/3gpp2"},
    {".7z",         "application/x-7z-compressed"}
};

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

            if(url[i] == '%' && (i + 2) < url.size())
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
    static std::string get_information_from_status(int status) 
    { 
        auto it = status_msg.find(status);
        if(it == status_msg.end())
            return "unknown status";
        return status_msg[status];
    }

    // 根据文件后缀名获取mime（即想要请求的文件格式）
    static std::string get_mime_from_suffix(const std::string& filename) 
    { 
        // 先获取后缀名
        size_t pos = filename.find_last_of('.');
        if(pos == std::string::npos)
            return "application/octet-stream";     // 错误或者没找到的话返回一个二进制流的格式
        std::string suffix = filename.substr(pos);

        // 再判断存不存在
        auto it = mime_msg.find(suffix);
        if(it == mime_msg.end())
            return "application/octet-stream";     // 错误或者没找到的话返回一个二进制流的格式
        return mime_msg[suffix];
    }

    // 判断一个文件是否是目录
    static bool is_directory(const std::string& filename)
    {
        // 通过stat函数获取文件的属性，然后通过S_ISDIR来判断其中的文件类型是否为目录即可
        struct stat buf = { 0 };
        int ret = stat(filename.c_str(), &buf);
        if(ret == -1)
            return false;
        
        return S_ISDIR(buf.st_mode);
    }

    // 判断一个文件是否是一个普通文件
    static bool is_regular_file(const std::string& filename)
    {
        // 通过stat函数获取文件的属性，然后通过S_ISREG来判断其中的文件类型是否为目录即可
        struct stat buf = { 0 };
        int ret = stat(filename.c_str(), &buf);
        if(ret == -1)
            return false;
        
        return S_ISREG(buf.st_mode);
    }

    // 判断一个HTTP资源路径是否有效
    //      /index.html  --- 前边的/叫做相对根目录，映射的是某个服务器上的子目录
    //      想表达的意思就是，客户端只能请求相对根目录中的资源，其他地方的资源都不予理会
    //      而/../login --- 这个路径中的..会让路径的查找跑到相对根目录之外，这是不合理的，不安全的
    static bool is_path_valid(const std::string& path)
    {
        // 1. 以斜杆/为分割字符，进行字符串分割
        std::vector<std::string> substr;
        split(path, "/", &substr);

        // 2. 然后遍历每个子串
        int level = 0; // 表示当前的层数
        for(int i = 0; i < substr.size(); ++i)
        {
            if(substr[i] == "..")
            {
                level--;
                if(level < 0)
                    return false; // 如果小于0说明访问到了根目录以外的内容，是不合法的，直接false
            }
            else
                level++; // 如果不是..的话，那么有多少个子串，就相当于进入了多少个目录，则让level++即可
        }
        return true;
    }
};

class HttpRequest
{
public:
    std::string _method;  // 请求方法
    std::string _path;    // 资源路径
    std::unordered_map<std::string, std::string> _queryString; // 查询字符串
    std::string _version; // 协议版本
    std::unordered_map<std::string, std::string> _header;      // 头部字段
    std::string _body;    // 请求正文

    std::smatch _matches; // 资源路径的正则提取数据
public:
    // 插入头部字段
    void set_header(const std::string& key, const std::string& val) {  _header[key] = val; }

    // 判断是否存在指定头部字段
    bool has_header(const std::string& key)
    {
        auto it = _header.find(key);
        if(it == _header.end())
            return false;
        return true;
    }

    // 获取指定头部字段的值
    std::string get_header_val(const std::string& key)
    {
        auto it = _header.find(key);
        if(it == _header.end())
            return "";
        return it->second;
    }

    // 插入查询字符串
    void set_queryString(const std::string& key, const std::string& val) {  _queryString[key] = val; }

    // 判断是否存在指定查询字符串
    bool has_queryString(const std::string& key)
    {
        auto it = _queryString.find(key);
        if(it == _queryString.end())
            return false;
        return true;
    }

    // 获取指定查询字符串的值
    std::string get_queryString_val(const std::string& key)
    {
        auto it = _queryString.find(key);
        if(it == _queryString.end())
            return "";
        return it->second;
    }

    // 获取正文长度
    size_t get_body_length()
    {
        // 通过头部字段中的Content-Length来获取，比如Content-Length: 1024\r\n
        bool ret = has_header("Content-Length");
        if(ret == false)
            return 0;
        return std::stol(get_header_val("Content-Length"));
    }

    // 判断是否为短连接
    bool is_short_connection()
    {
        // 通过头部字段中的Connection来判断，如果是close表示短连接，keep-alive表示长连接
        bool ret = has_header("Connection");
        if(ret == false)
            return 0;
        return get_header_val("Connection") == "close";
    }
public:
    // 成员变量清理接口
    void reset()
    {
        _method.clear();
        _path.clear();
        _queryString.clear();
        _version.clear();
        _header.clear();
        _body.clear();
        
        // smatch比较特殊，它没有clear()接口，所以我们可以用一个空的smatch与其进行交换达到清空的效果
        std::smatch tmp;
        _matches.swap(tmp);
    }
};