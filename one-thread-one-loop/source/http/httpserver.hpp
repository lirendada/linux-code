#include "../server.hpp"
#include <fstream>
#include <ctype.h>

const int DEFAULT_TIMEOUT = 10;

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
    HttpRequest()
        : _version("HTTP/1.1")
    {}

    // 插入头部字段
    void set_header(const std::string& key, const std::string& val) {  _header[key] = val; }

    // 判断是否存在指定头部字段
    bool has_header(const std::string& key) const
    {
        auto it = _header.find(key);
        if(it == _header.end())
            return false;
        return true;
    }

    // 获取指定头部字段的值
    std::string get_header_val(const std::string& key) const
    {
        auto it = _header.find(key);
        if(it == _header.end())
            return "";
        return it->second;
    }

    // 插入查询字符串
    void set_queryString(const std::string& key, const std::string& val) {  _queryString[key] = val; }

    // 判断是否存在指定查询字符串
    bool has_queryString(const std::string& key) const
    {
        auto it = _queryString.find(key);
        if(it == _queryString.end())
            return false;
        return true;
    }

    // 获取指定查询字符串的值
    std::string get_queryString_val(const std::string& key) const
    {
        auto it = _queryString.find(key);
        if(it == _queryString.end())
            return "";
        return it->second;
    }

    // 获取正文长度
    size_t get_body_length() const 
    {
        // 通过头部字段中的Content-Length来获取，比如Content-Length: 1024\r\n
        bool ret = has_header("Content-Length");
        if(ret == false)
            return 0;
        return std::stol(get_header_val("Content-Length"));
    }

    // 判断是否为短连接
    bool is_short_connection() const
    {
        // 通过头部字段中的Connection来判断，如果是close表示短连接，keep-alive表示长连接
        bool ret = has_header("Connection");
        if(ret == false)
            return false;
        return get_header_val("Connection") == "close";
    }
public:
    // 成员变量清理接口
    void reset()
    {
        _method.clear();
        _path.clear();
        _queryString.clear();
        _version = "HTTP/1.1";
        _header.clear();
        _body.clear();
        
        // smatch比较特殊，它没有clear()接口，所以我们可以用一个空的smatch与其进行交换达到清空的效果
        std::smatch tmp;
        _matches.swap(tmp);
    }
};

class HttpResponse
{
public:
    int _status;                                          // 状态码
    std::string _body;                                    // 响应正文
    std::unordered_map<std::string, std::string> _header; // 头部字段

    bool _is_redirect;          // 是否重定向的标志
    std::string _redirect_path; // 重定向路径
public:
    HttpResponse(int status = 200)
        : _is_redirect(false)
        , _status(status)
    {}

    // 成员变量清理接口
    void reset()
    {
        _status = 200;
        _is_redirect = false;
        _body.clear();
        _header.clear();
        _redirect_path.clear();
    }

    // 插入头部字段
    void set_header(const std::string& key, const std::string& val) {  _header[key] = val; }

    // 判断是否存在指定头部字段
    bool has_header(const std::string& key) const 
    {
        auto it = _header.find(key);
        if(it == _header.end())
            return false;
        return true;
    }

    // 获取指定头部字段的值
    std::string get_header_val(const std::string& key) const 
    {
        auto it = _header.find(key);
        if(it == _header.end())
            return "";
        return it->second;
    }

    // 设置响应正文
    void set_content(const std::string& body, const std::string& type = "text/html")
    {
        _body = body;
        set_header("Content-Type", type);
    }

    // 设置重定向信息
    void set_redirect(const std::string& url, int status = 302)
    {
        _status = status;
        _is_redirect = true;
        _redirect_path = url;
    }

    // 判断是否为短连接
    bool is_short_connection() const 
    {
        // 通过头部字段中的Connection来判断，如果是close表示短连接，keep-alive表示长连接
        bool ret = has_header("Connection");
        if(ret == false)
            return 0;
        return get_header_val("Connection") == "close";
    }
};

// RECV_LINE：接收请求行（当前处于接收并处理请求行的阶段）
// RECV_HEADER：接收请求头部（表示请求头部的接收还没有完毕）
// RECV_BODY：接收正文（表示还有正文没有接收完毕）
// RECV_DONED：接收数据完毕（这是一个接收完毕，可以对请求进行处理的阶段）
// RECV_ERROR：接收处理请求出错
typedef enum {
    RECV_LINE,      
    RECV_HEADER,    
    RECV_BODY,      
    RECV_DONED,     
    RECV_ERROR      
} HTTP_RECV_STATUS;

const int MAX_LINE_SIZE = 8192;
class HttpContext
{
private:
    int _response_status;          // 响应状态码
    HTTP_RECV_STATUS _recv_status; // 当前接收的阶段
    HttpRequest _request;          // 存放已经接收并处理的请求信息
public:
    HttpContext()
        : _response_status(200)
        , _recv_status(RECV_LINE)
    {}

    // 获取响应状态码
    int get_response_status() { return _response_status; }
    
    // 获取接收解析状态
    HTTP_RECV_STATUS get_recv_status() { return _recv_status; }

    // 获取解析完毕的请求信息
    HttpRequest& get_request() { return _request; }

    // 接收并处理请求数据
    void recv_and_handle_request(Buffer* buffer)
    {
        // 不同的状态，做不同的事情，但是这里不能break，因为处理完请求行后，应该立即处理头部，而不是退出等新数据
        switch(_recv_status)
        {
            case RECV_LINE: recv_line(buffer);
            case RECV_HEADER: recv_header(buffer);
            case RECV_BODY: recv_body(buffer);
        }
    }

    void reset()
    {
        _response_status = 200;
        _recv_status = RECV_LINE;
        _request.reset();
    }
private:
    // 接收请求行
    bool recv_line(Buffer* buffer)
    {
        // 1. 接收请求行之前，判断当前是否处于接收请求行的阶段
        if(_recv_status != RECV_LINE)
            return false;

        // 2. 获取缓冲区中的一行
        std::string line = buffer->get_line_andMove();

        // 3. 判断两种特殊情况：请求行没有读取完毕、请求行超过服务器规定（一般是8K）
        if(line.size() == 0)
        {
            /* 如果此时请求行没有读取完毕，而且缓冲区中的数据是超过MAX_LINE_SIZE的，
               说明数据很长都不足一行，这已经是有问题的了，那么请求行肯定是超过MAX_LINE_SIZE了 */
            if(buffer->get_sizeof_read() > MAX_LINE_SIZE)
            {
                _recv_status = RECV_ERROR;
                _response_status = 414; // 414表示URI太长了
                return false;
            }
            return true; // 返回true表示没有读取完毕，不算错误
        }

        if(line.size() > MAX_LINE_SIZE)
        {
            _recv_status = RECV_ERROR;
            _response_status = 414; // 414表示URI太长了
            return false;
        }

        // 4. 获取成功的话则调用parse_line()开始解析请求行（其内部会解析完将各字段放到请求模块对象中）
        bool ret = parse_line(line);
        if(ret == false)
            return false;
        
        // 5. 将所处状态改为接收头部状态
        _recv_status = RECV_HEADER;
        return true;
    }

    // 解析请求行（内部会解析完将各字段放到请求模块对象中）
    bool parse_line(const std::string& line)
    {
        // (GET|POST|HEAD|PUT|DELETE)   表示匹配并提取其中任意一个字符串
        // [^?]*                        [^?] 匹配非问号字符，后边的*表示 0次或多次
        // \\?(.*)                      \\? 表示原始的 ? 字符，(.*)表示提取 ? 之后的任意字符 0 次或多次，直到遇到空格
        // (?:\\?(.*))?                 (?: ...) 表示匹配某个格式字符串，但是不提取，所以就是表示匹配了上一行注释 0 次或 1 次，并且不获取该内容
        // HTTP/1\\.[01]                表示匹配以 HTTP/1. 开始，后边有个 0 或 1 的字符串
        // (?:\n|\r\n)?                 (?: ...) 表示匹配某个格式字符串，但是不提取，而最后的 ? 表示的是匹配前边的表达式 0 次或 1 次
        std::regex rule("(GET|POST|HEAD|PUT|DELETE) ([^?]*)(?:\\?(.*))? (HTTP/1\\.[01])(?:\n|\r\n)?", std::regex::icase); // 要使用icase表示忽略大小写
        std::smatch matches; // 结果集

        bool ret = std::regex_match(line, matches, rule); // 进行表达式匹配，将匹配结果放到结果集中
        if (ret == false) 
        {
            _recv_status = RECV_ERROR;
            _response_status = 400; // BAD REQUEST
            return false;
        }

        // 举个例子，此时"GET /liren/login?user=xiaoming&pass=123123 HTTP/1.1\r\n" 的结果如下所示：
        //      0 : GET /liren/login?user=xiaoming&pass=123123 HTTP/1.1
        //      1 : GET
        //      2 : /liren/login
        //      3 : user=xiaoming&pass=123123
        //      4 : HTTP/1.1

        // 1. 请求方法的获取
        _request._method = matches[1];
        std::transform(_request._method.begin(), _request._method.end(), _request._method.begin(), ::toupper); // 注意要将方法转化为大写

        // 2. 资源路径的获取，需要对其进行url解码，但是不需要将+转化为空格
        _request._path = Util::url_decode(matches[2], false);

        // 3. 协议版本的获取
        _request._version = matches[4];

        // 4.1 查询字符串的获取，先获取每个key=val的结构也就是键值对组合
        std::vector<std::string> strs;
        int size = Util::split(matches[3], "&", &strs);

        // 4.2 然后再分解获取每个key和val
        for(int i = 0; i < size; ++i)
        {
            std::vector<std::string> key_val;
            int n = Util::split(strs[i], "=", &key_val);

            // 此时如果只有key没有val的话则是错误的
            if(n == 1)
            {
                _recv_status = RECV_ERROR;
                _response_status = 400;
                return false;
            }

            // 正确获取的话则对key和val先进行url解析，此时就需要将+转化为空格，然后将它们设置进请求对象中保存
            std::string key = Util::url_decode(key_val[0], true);
            std::string val = Util::url_decode(key_val[1], true);
            _request.set_queryString(key, val);
        }
        return true;
    }

    // 接收头部（大部分和上面的接收请求行是重合的，注意不同的地方即可）
    bool recv_header(Buffer* buffer)
    {
        // 1. 接收头部之前，判断当前是否处于接收头部的阶段
        if(_recv_status != RECV_HEADER)
            return false;
        
        // 因为头部有多行，所以要用死循环
        while(true)
        {
            // 2. 获取缓冲区中的一行
            std::string line = buffer->get_line_andMove();

            // 3. 判断两种特殊情况：一行没有读取完毕、请求行超过服务器规定（一般是8K）
            if(line.size() == 0)
            {
                /* 如果此时请求行没有读取完毕，而且缓冲区中的数据是超过MAX_LINE_SIZE的，
                说明数据很长都不足一行，这已经是有问题的了，那么请求行肯定是超过MAX_LINE_SIZE了 */
                if(buffer->get_sizeof_read() > MAX_LINE_SIZE)
                {
                    _recv_status = RECV_ERROR;
                    _response_status = 414; // 表示一行数据太多
                    return false;
                }
                return true; // 返回true表示没有读取完毕，不算错误
            }

            if(line.size() > MAX_LINE_SIZE)
            {
                _recv_status = RECV_ERROR;
                _response_status = 414; // 表示一行数据太多
                return false;
            }

            // 4. 如果读的头部是\n或者\r\n的话，表示头部接收结束了，则将所处状态改为接收正文状态，然后退出循环，
            if(line == "\r\n" || line == "\n")
                break;

            // 5. 获取成功的话则调用parse_line()开始解析请求行（其内部会解析完将各字段放到请求模块对象中）
            bool ret = parse_header(line);
            if(ret == false)
                return false;
        }
        _recv_status = RECV_BODY;
        return true;
    }

    // 解析头部
    bool parse_header(std::string& line)
    {
        // 1. 末尾是\n或者\r换行则要去掉
        if (line.back() == '\n') line.pop_back(); 
        if (line.back() == '\r') line.pop_back(); 

        // 2. 根据key: val的格式，进行分割获取头部的key和val
        std::vector<std::string> key_val;
        int size = Util::split(line, ": ", &key_val);
        if(size <= 1)
        {
            _recv_status = RECV_ERROR;
            _response_status = 400; 
            return false;
        }
        
        // 3. 将key和val设置进请求对象中保存
        _request.set_header(key_val[0], key_val[1]);
        return true;
    }

    // 接收正文
    bool recv_body(Buffer* buffer)
    {
        // 1. 接收正文之前，判断当前是否处于接收正文的阶段
        if(_recv_status != RECV_BODY)
            return false;
        
        // 2. 从头部中获取正文长度
        size_t size = _request.get_body_length();
        if(size == 0)
        {
            // 没有正文，则请求解析完毕
            _recv_status = RECV_DONED;
            return true;
        }

        // 3. 计算还需要接收的正文长度（因为可能前面因为数据只接收了部分）
        size_t real_length = size - _request._body.size();
        if(buffer->get_sizeof_read() >= real_length)
        {
            // 3.1 若缓冲区中的数据包含了当前请求的所有正文，则取出所需的数据，然后设置状态为接收完毕即可
            _request._body.append(buffer->start_of_read(), real_length);
            buffer->push_reader_back(real_length);
            _recv_status = RECV_DONED;
            return true;
        }
        
        // 3.2 若缓冲区中的数据无法满足当前正文的需要，也就是数据不足，则取出数据，然后等待新数据到来，不用修改接收状态
        _request._body.append(buffer->start_of_read(), buffer->get_sizeof_read());
        buffer->push_reader_back(buffer->get_sizeof_read());
        return true;
    }
};

using handle_t = std::function<void(const HttpRequest&, HttpResponse*)>;
class HttpServer
{
private:
    TcpServer _server;             // 高性能服务器对象
    std::string _static_directory; // 静态资源根目录

    std::vector<std::pair<std::regex, handle_t>> get_route;    // get方法的执行函数路由表
    std::vector<std::pair<std::regex, handle_t>> post_route;   // post方法的执行函数路由表
    std::vector<std::pair<std::regex, handle_t>> put_route;    // put方法的执行函数路由表
    std::vector<std::pair<std::regex, handle_t>> delete_route; // delete方法的执行函数路由表
public:
    HttpServer(uint16_t port, int timeout = DEFAULT_TIMEOUT)
        : _server(port)
    {
        _server.set_connected_callback(std::bind(&HttpServer::connected_handle, this, std::placeholders::_1));
        _server.set_message_callback(std::bind(&HttpServer::message_handle, this, std::placeholders::_1, std::placeholders::_2));
        _server.enable_inactive_release(timeout);
    }

    // 添加请求-处理函数的映射信息接口（注意这里key不是字符串，而是一个正则表达式）
    void add_get(const std::string& pattern, const handle_t& handler) { get_route.push_back(std::make_pair(std::regex(pattern), handler)); }
    void add_post(const std::string& pattern, const handle_t& handler) { post_route.push_back(std::make_pair(std::regex(pattern), handler)); }
    void add_put(const std::string& pattern, const handle_t& handler) { put_route.push_back(std::make_pair(std::regex(pattern), handler)); }
    void add_delete(const std::string& pattern, const handle_t& handler) { delete_route.push_back(std::make_pair(std::regex(pattern), handler)); }

    // 设置静态资源根目录接口
    void set_static_directory(const std::string& path) 
    {
        assert(Util::is_directory(path) == true);
        _static_directory = path; 
    }

    // 设置是否启动非活跃连接超时关闭接口
    void enable_inactive_release(int timeout) { _server.enable_inactive_release(timeout); }

    // 设置线程池中线程数量接口
    void set_nums_of_thread(int count) { _server.set_nums_of_subthread(count); }

    // 启动服务器接口
    void start_httpserver() { _server.start_server(); }
private:
    // 连接建立完成后的回调处理
    void connected_handle(const ConnectionPtr& cptr) 
    { 
        cptr->set_context(HttpContext()); 
        DLOG("new connection: %p", cptr.get());
    }

    // 收到消息后的回调处理
    void message_handle(const ConnectionPtr& cptr, Buffer* buffer)
    {
        //DLOG("message_handle, size is %d", buffer->get_sizeof_read());
        // 如果缓冲区有数据的话就进行持续的处理
        while(buffer->get_sizeof_read() > 0)
        {
            // 1. 获取上下文
            HttpContext* context = cptr->get_context()->get<HttpContext>();

            // 2. 通过上下文对缓冲区数据进行解析，得到HttpRequest对象（如果缓冲区数据解析成功，且请求已经获取完毕了，才开始去路由查找和处理）
            context->recv_and_handle_request(buffer);

            //  2.1 如果缓冲区数据解析失败，则直接响应错误信息然后关闭连接即可
            HttpRequest& request = context->get_request();
            HttpResponse response(context->get_response_status());
            if(context->get_response_status() >= 400)
            {
                // 即填充错误信息页面数据到响应中，然后返回该错误页面响应
                error_response(cptr, request, &response);

                // 出错了就把当前连接的缓冲区数据清空，不然会和下面的shutdown函数形成死循环。最后顺便把状态也清空一下
                request.reset();        
                buffer->clear_buffer(); 
                
                cptr->shutdown();
                return;
            }

            //  2.2 如果缓冲区数据解析成功，但是请求还没获取完整，则退出该函数，等待新数据的到来后再继续处理
            if(context->get_recv_status() != RECV_DONED)
                return;

            // 3. 进行路由查找（在其内部进行对应请求的处理）
            route(request, &response);
            
            // 4. 组织HttpResponse进行返回
            organize_and_response(cptr, request, &response);

            // 5. 重置上下文，防止下一条请求被影响（比如状态码什么的）
            context->reset();

            // 6. 判断是否为短连接，是的话直接关闭连接
            if(request.is_short_connection())
                cptr->shutdown();
        }
    }

    // 路由查找函数
    void route(HttpRequest& request, HttpResponse* response)
    {
        // 1. 如果是静态资源请求的话，则进行静态资源请求处理
        if(is_static_resource_request(request) == true)
            return static_resource_request(request, response);
        
        // 2. 如果是功能性请求的话，则根据请求方法来将不同功能性请求派发到不同作用的函数中去
        if(request._method == "GET" || request._method == "HEAD")
            return functional_request(request, response, get_route);
        else if(request._method == "POST")
            return functional_request(request, response, post_route);
        else if(request._method == "PUT")
            return functional_request(request, response, put_route);
        else if(request._method == "DELETE")
            return functional_request(request, response, delete_route);

        // 3. 如果既不是静态资源请求，也不是功能性请求的话，则设置状态码为405表示请求方法未找到
        response->_status = 405;
    }

    // 判断是否为正确的静态资源请求
    bool is_static_resource_request(const HttpRequest& request)
    {
        // 1. 要求必须设置了静态资源根目录
        if(_static_directory.empty())
            return false;

        // 2. 要求请求方法GET或者HEAD
        if(request._method != "GET" && request._method != "HEAD")
            return false;
        
        // 3. 判断请求路径是否合法
        if(Util::is_path_valid(request._path) == false)
            return false;
        
        // 4. 请求的资源必须存在，并且是一个普通文件
        //    如果是请求资源是目录的话，那么就在其后面加上默认页面文件index.html即可
        std::string path = _static_directory + request._path; // 为了避免直接修改请求的资源路径，因此定义一个临时对象
        if(path.back() == '/')
            path += "index.html";
        if(Util::is_regular_file(path) == false)
            return false;
        return true;
    }

    // 静态资源请求处理函数
    void static_resource_request(const HttpRequest& request, HttpResponse* response)
    {
        // 就是将要请求的静态资源读取出来后，放到response的正文中，然后设置资源的类型Content-Type即可
        std::string path = _static_directory + request._path;
        if(path.back() == '/')
            path += "index.html";

        bool ret = Util::read_file(path, &response->_body);
        if(ret == false)
            return;
        response->set_header("Content-Type", Util::get_mime_from_suffix(path));
    }
 
    // 功能性请求函数的分类处理
    void functional_request(HttpRequest& request, HttpResponse* response, std::vector<std::pair<std::regex, handle_t>>& route)
    {
        // 在对应请求方法的路由表中，查找是否存在该请求的处理方法，如果有的话则调用，没有的话则设置404状态码
        //   思路：使用路由表中的每个正则表达式与请求路径进行匹配，匹配成功则使用对应函数进行处理
        //         所以路由表中的key最好存放的是正则表达式，如果是字符串的话则需要去编译成正则表达式，比较费时间
        for(auto& handler : route)
        {
            const std::regex& regex = handler.first;
            bool ret = std::regex_match(request._path, request._matches, regex);
            if(ret == false)
                continue;
            
            return handler.second(request, response); // 传入请求信息和空的response，执行对应的处理函数
        }

        // 如果走到这里的话，说明上面的功能性请求没找到对应的处理方法，则设置404状态码即可
        response->_status = 404;
    }

    // 组织协议格式进行返回的函数
    void organize_and_response(const ConnectionPtr& cptr, const HttpRequest& request, HttpResponse* response)
    {
        // 1. 完善一些头部字段（比如长短连接、正文长度、请求资源类型、重定向等等）
        if(request.is_short_connection() == true)
            response->set_header("Connection", "close");
        else
            response->set_header("Connection", "keep-alive");

        if(!response->_body.empty() && response->has_header("Content-Length") == false)
            response->set_header("Content-Length", std::to_string(response->_body.size()));
        
        if(!response->_body.empty() && response->has_header("Content-Type") == false)
            response->set_header("Content-Type", "application/octet-stream");
        
        if(response->_is_redirect == true)
            response->set_header("Location", response->_redirect_path);

        // 2. 组织响应内容（状态行、响应报头、空行、响应正文）
        std::stringstream sstr;
        sstr << request._version << " " << std::to_string(response->_status) << " " << Util::get_information_from_status(response->_status) << "\r\n";
        for(auto& e : response->_header)
            sstr << e.first << ": " << e.second << "\r\n";
        sstr << "\r\n" << response->_body;
        
        // 3. 发送数据
        cptr->send_data(sstr.str().c_str(), sstr.str().size());
    }

    // 响应错误信息
    void error_response(const ConnectionPtr& cptr, const HttpRequest& request, HttpResponse* response)
    {
        // 1. 读取错误页面文件
        std::string buffer;
        bool ret = Util::read_file("./wwwroot/error.html", &buffer);
        if(ret == false)
        {
            ELOG("响应错误信息操作失败！");
            return;
        }

        // 2. 将数据设置为响应正文，然后进行组织发送
        response->set_content(buffer, "text/html");
        organize_and_response(cptr, request, response);
    }
};