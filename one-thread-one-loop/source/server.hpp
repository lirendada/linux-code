#ifndef __MY_SERVER_H__
#define __MY_SERVER_H__
#include <iostream>
#include <vector>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <functional>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <time.h>

const static uint64_t MAX_BUFFER_SIZE = 1024;

#define INF 0    // 提示型等级
#define DEBUG 1  // 调试型等级
#define ERROR 2  // 错误型等级
#define DEFAULT_LOG_LEVEL DEBUG  // 默认的日志等级

#define LOG(level, format, ...) do{\
    if(DEFAULT_LOG_LEVEL > level) break;\
    char timebuffer[128];\
    time_t timestamp = time(NULL);\
    struct tm* timeinfo = localtime(&timestamp);\
    strftime(timebuffer, sizeof(timebuffer), "%Y-%m-%d %H:%M:%S", timeinfo);\
    fprintf(stdout, "[%s %s:%d] " format "\n", timebuffer, __FILE__, __LINE__, ##__VA_ARGS__);\
}while(0)

// 将等级和日志打印封装起来
#define ILOG(format, ...) LOG(INF,   format, ##__VA_ARGS__)
#define DLOG(format, ...) LOG(DEBUG, format, ##__VA_ARGS__)
#define ELOG(format, ...) LOG(ERROR, format, ##__VA_ARGS__)

class Buffer
{
private:
    std::vector<char> _buffer; // 缓冲区
    uint64_t _reader;	   // 当前读位置
    uint64_t _writer;      // 当前写位置
public:
    Buffer() :_reader(0), _writer(0), _buffer(MAX_BUFFER_SIZE) {}
    
    /////////////////////////////  写操作 /////////////////////////////
    // 1. 获取当前写入的起始地址
    char* start_of_write()
    {
        return &(*_buffer.begin()) + _writer;
    }

    // 2. 获取可写数据的大小
    uint64_t get_sizeof_write()
    {
        // 即开头空闲空间 + 结尾空闲空间
        return get_head_free_size() + get_tail_free_size();
    }

    // 3. 将写位置向后移动
    void push_writer_back(uint64_t size)
    {
        // 向后移动的大小，必须小于当前后边的空闲空间大小
        assert(size <= get_tail_free_size());
        _writer += size;
    }

    // 4. 写入数据操作（将data中数据写到缓冲区中）
    void write_data(const void* data, uint64_t size)
    {
        // 1. 确保空间足够
        if(size == 0)
            return;
        ensure_write_space(size);

        // 2. 将数据拷贝到缓冲区，注意这里需要转换类型
        const char* tmp = (const char*)data;
        std::copy(tmp, tmp + size, start_of_write());
    }

    // 5. 写入数据操作，并让写入位置向后移动
    void write_data_andMove(const void* data, uint64_t size)
    {
        write_data(data, size);
        push_writer_back(size);
    }

    // 6. 写入string类型数据操作
    void write_string(const std::string& data)
    {
        return write_data(data.c_str(), data.size());
    }

    // 7. 写入string类型数据操作，并让写入位置向后移动
    void write_string_andMove(const std::string& data)
    {
        write_string(data);
        push_writer_back(data.size());
    }

    // 8. 写入Buffer类型数据操作
    void write_Buffer(Buffer& data)
    {
        return write_data(data.start_of_read(), data.get_sizeof_read());
    }

    // 9. 写入Buffer类型数据操作，并让写入位置向后移动
    void write_Buffer_andMove(Buffer& data)
    {
        write_Buffer(data);
        push_writer_back(data.get_sizeof_read());
    }

    // 10. 确保可写空间足够大（整体空闲空间够了就移动数据，否则就扩容）
    void ensure_write_space(uint64_t size)
    {
        // 如果末尾空闲空间大小足够，直接返回
        if(size <= get_tail_free_size())
            return;

        // 末尾空闲空间不够，则判断加上起始位置的空闲空间大小是否足够, 够了就将数据移动到起始位置
        if(size <= get_sizeof_write())
        {
            // 1. 先记录下可读数据的大小，防止下面搬移后变了
            uint64_t read_size = get_sizeof_read();

            // 2. 将可读部分的数据都搬到数组开头
            std::copy(start_of_read(), start_of_read() + read_size, &(*_buffer.begin()));
            
            // 3. 然后重新设置读写位置
            _reader = 0;
            _writer = read_size;
        }
        else
        {
            // 总体空闲空间不够，则需要扩容，不移动数据，直接在写偏移之后扩容足够空间即可
            _buffer.resize(_writer + size);
        }
    }

    /////////////////////////////  读操作 /////////////////////////////
    // 1. 获取当前读取的起始地址
    char* start_of_read()
    {
        return &(*_buffer.begin()) + _reader;
    }

    // 2. 获取可读数据的大小
    uint64_t get_sizeof_read()
    {
        // 可读数据的大小 = 写偏移 - 读偏移
        return _writer - _reader;
    }

    // 3. 将读位置向后移动
    void push_reader_back(uint64_t size)
    {
        // 向后移动的大小，必须小于可读数据大小
        assert(size <= get_sizeof_read());
        _reader += size;
    }

    // 4. 读取数据操作（读取到buffer中）
    void read_data(char* buffer, uint64_t size)
    {
        // 1. 要求要获取的数据大小必须小于可读数据大小
        assert(size <= get_sizeof_read());

        // 2. 读取数据到buffer中
        std::copy(start_of_read(), start_of_read() + size, (char*)buffer);
    }

    // 5. 读取数据操作（读取到buffer中），并将读取位置向后移动
    void read_data_andMove(char* buffer, uint64_t size)
    {
        read_data(buffer, size);
        push_reader_back(size);
    }

    // 6. 读取数据后转化为string类型返回
    std::string read_to_string(uint64_t size)
    {
        // 1. 要求要获取的数据大小必须小于可读数据大小
        assert(size <= get_sizeof_read());

        // 2. 调用read_data接口进行读写
        std::string str;
        str.resize(size);
        read_data(&str[0], size); // 传参时候不能使用c_str()，因为其类型是const的
        return str;
    }

    // 7. 读取数据后转化为string类型返回，并且让读取位置向后移动
    std::string read_to_string_andMove(uint64_t size)
    {
        std::string str = read_to_string(size);
        push_reader_back(size);
        return str;
    }

    /////////////////////////////  其它操作 /////////////////////////////
    // 1. 清空缓冲区接口
    void clear_buffer()
    {
        // 只需要将偏移量置零即可
        _writer = _reader = 0;
    }

    // 2. 找到换行的位置（方便http解析）
    char* find_CRLF()
    {
        // 这里我们使用memchr找到\n就算找到换行位置
        char* res = (char*)memchr(start_of_read(), '\n', get_sizeof_read());
        return res;
    }
    
    // 3. 获取一行数据（方便http解析）
    std::string get_line()
    {
        char* pos = find_CRLF();
        if(pos == nullptr)
            return "";
        
        // 这里+1是为了把换行字符也取出来
        return read_to_string(pos - start_of_read() + 1);
    }

    // 4. 获取一行数据然后移动读取下标
    std::string get_line_andMove()
    {
        std::string str = get_line();
        push_reader_back(str.size());
        return str;
    }
private:
    // 1. 获取缓冲区末尾空闲空间的大小 -- 即写位置之后的空闲空间
    uint64_t get_tail_free_size()
    {
        return _buffer.size() - _writer;
    }

    // 2. 获取缓冲区开头空闲空间的大小 -- 即读位置之前的空闲空间
    uint64_t get_head_free_size()
    {
        return _reader;
    }
};


const static int MAXBACKLOG = 1024;
class Socket
{
private:
    int _sockfd;
public:
    Socket() : _sockfd(-1) {}
    Socket(int sockfd) : _sockfd(sockfd) {}
    ~Socket() { Close(); };

    // 1. 创建套接字
    bool Create()
    {
        _sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // IPPROTO_TCP指定的是TCP协议
        if(_sockfd < 0)
        {
            ELOG("create socket error!!");
            return false;
        }
        return true;
    }

    // 2. 绑定套接字信息
    bool Bind(const std::string& ip, uint16_t port)
    {
        // 填充信息
        struct sockaddr_in local;
        memset(&local, 0, sizeof(struct sockaddr_in));
        local.sin_family = AF_INET;
        local.sin_port = htons(port);
        local.sin_addr.s_addr = inet_addr(ip.c_str());

        // 绑定信息
        int n = bind(_sockfd, (struct sockaddr*)&local, sizeof(struct sockaddr_in));
        if(n < 0)
        {
            ELOG("bind error!!");
            return false;
        }
        return true;
    }

    // 3. 监听套接字
    bool Listen(int backlog = MAXBACKLOG)
    {
        int n = listen(_sockfd, backlog);
        if(n < 0)
        {
            ELOG("listen error!!");
            return false;
        }
        return true;
    }

    // 4. 获取新连接
    int Accept()
    {
        // 这里不关心获取到的新连接的属性
        int newfd = accept(_sockfd, nullptr, nullptr);
        if(newfd < 0)
        {
            ELOG("accept error!!");
            return -1;
        }
        return newfd;
    }

    // 5. 客户端发起请求
    bool Connect(const std::string& ip, uint16_t port)
    {
        // 填充信息
        struct sockaddr_in client;
        memset(&client, 0, sizeof client);
        client.sin_family = AF_INET;
        client.sin_port = htons(port);
        client.sin_addr.s_addr = inet_addr(ip.c_str());

        // 发起连接
        int n = connect(_sockfd, (struct sockaddr*)&client, sizeof(struct sockaddr_in));
        if(n < 0)
        {
            ELOG("connect server error!!");
            return false;
        }
        return true;
    }

    // 6. 接收数据
    ssize_t Send(const void* buffer, size_t size, int flag = 0)
    {
        ssize_t n = send(_sockfd, buffer, size, flag);
        if(n < 0)
        {
            // EAGAIN 表示当前socket的发送缓冲区满了，在非阻塞的情况下才会有这个错误
            // EINTR  表示当前socket的阻塞等待，被信号打断了
            if(errno == EAGAIN || errno == EINTR)
                return 0;
            ELOG("send error");
            return -1;
        }
        return n; // 返回实际发送的数据长度
    }

    // 非等待式发送数据
    ssize_t send_with_noblock(const void* buffer, size_t size)
    {
        if(size == 0)
            return 0;
        return Send(buffer, size, MSG_DONTWAIT); // MSG_DONTWAIT 表示当前接收为非阻塞
    }

    // 7. 发送数据
    ssize_t Recv(void* buffer, size_t size, int flag = 0)
    {
        ssize_t n = recv(_sockfd, buffer, size, flag);
        if(n < 0)
        {
            // EAGAIN 表示当前socket的接收缓冲区中没有数据了，在非阻塞的情况下才会有这个错误
            // EINTR  表示当前socket的阻塞等待，被信号打断了
            if(errno == EAGAIN || errno == EINTR)
                return 0;
            ELOG("recv error");
            return -1;
        }
        return n; // 返回实际发送的数据长度
    }

    // 非等待式接收数据
    ssize_t recv_with_noblock(void* buffer, size_t size)
    {
        return Recv(buffer, size, MSG_DONTWAIT); // MSG_DONTWAIT 表示当前接收为非阻塞
    }

    // 8. 关闭套接字
    void Close()
    {
        if(_sockfd != -1)
        {
            close(_sockfd);
            _sockfd = -1;
        }
    }

    // 9. 创建一个服务端链接的接口
    bool create_server(uint16_t port, const std::string& ip = "0.0.0.0", bool isNonBlock = false)
    {
        // 1. 创建套接字  2. 绑定地址  3. 开始监听  4. 设置非阻塞  5. 启动地址重用
        if(Create() == false) 
            return false;
        if(Bind(ip, port) == false) 
            return false;
        if(Listen() == false) 
            return false;
        if(isNonBlock)
            set_nonblock();
        reuse_addr();
        return true;
    }

    // 10. 创建一个客户端连接的接口
    bool create_client(uint16_t port, const std::string& ip)
    {
        // 1. 创建套接字  2. 发起连接
        if(Create() == false) 
            return false;
        if(Connect(ip, port) == false)
            return false;
        return true;
    }

    // 11. 设置套接字选项 -- 开启地址端口复用
    void reuse_addr()
    {
        // 设置地址复用
        int val = 1;
        setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, (void*)&val, sizeof(int));

        // 设置端口复用
        val = 1;
        setsockopt(_sockfd, SOL_SOCKET, SO_REUSEPORT, (void*)&val, sizeof(int));
    }

    // 12. 设置套接字阻塞属性 -- 设置为非阻塞
    void set_nonblock()
    {
        int old = fcntl(_sockfd, F_GETFL, 0);
        fcntl(_sockfd, F_SETFL, old | O_NONBLOCK);
    }
};


using eventcallback_t  = std::function<void()>; // 事件触发的函数类型
class Channel
{
private:
    int _fd;           // 文件描述符
    uint32_t _events;  // 当前需要监控的事件
    uint32_t _revents; // 当前触发或者就绪的事件（由外部设置）

    eventcallback_t _read_callback;       // 可读事件被触发的回调函数
    eventcallback_t _write_callback;      // 可写事件被触发的回调函数
    eventcallback_t _error_callback;      // 错误事件被触发的回调函数
    eventcallback_t _close_callback;      // 关闭事件被触发的回调函数
    eventcallback_t _arbitrary_callback;  // 任意事件被触发的回调函数
public:
    Channel(int fd) 
        : _fd(fd), _events(0), _revents(0) 
    {}

    int get_fd() { return _fd; }                               // 获取文件描述符
    uint32_t get_events() { return _events; }                  // 获取当前监控的事件
    void set_revents(uint32_t revents) { _revents = revents; } // 设置实际就绪的事件

    // 设置对应触发事件的回调函数
    void set_read_callback(const eventcallback_t& cb) { _read_callback = cb; }
    void set_write_callback(const eventcallback_t& cb) { _write_callback = cb; }
    void set_error_callback(const eventcallback_t& cb) { _error_callback = cb; }
    void set_close_callback(const eventcallback_t& cb) { _close_callback = cb; }
    void set_arbitrary_callback(const eventcallback_t& cb) { _arbitrary_callback = cb; }

    bool _is_read_able()  { return (_events & EPOLLIN); }    // 当前是否监控了可读
    bool _is_write_able() { return (_events & EPOLLOUT); }  // 当前是否监控了可写

    // 启动读事件监控
    void _enable_read() 
    {
        _events |= EPOLLIN; 
        // 其实这里还需要将读事件添加到EventLoop中管理，但是还没实现，所以这里就先留着
        // TODO
    }

     // 启动写事件监控
    void _enable_write() 
    {
        _events |= EPOLLOUT; 
        // 其实这里还需要将写事件添加到EventLoop中管理，但是还没实现，所以这里就先留着
        // TODO
    }

    // 关闭读事件监控
    void _disable_read()
    {
        _events &= (~EPOLLIN);
        // 其实这里还需要将读事件从EventLoop中移除，但是还没实现，所以这里就先留着
        // TODO
    }

    // 关闭写事件监控
    void _disable_write()
    {
        _events &= (~EPOLLOUT);
        // 其实这里还需要将写事件从EventLoop中移除，但是还没实现，所以这里就先留着
        // TODO
    }

    // 关闭所有事件监控
    void _disable_all()
    {
        _events = 0;
        // 其实这里还需要将所有事件从EventLoop中移除，但是还没实现，所以这里就先留着
        // TODO
    } 

    // 事件总处理函数。一旦触发了事件，就调用这个函数，而触发了什么事件如何处理由连接管理者决定
    void _handler()  
    {
        // 下面因为错误和关闭事件触发的时候会释放连接，此时就不能再调用_arbitrary_callback了，所以需要提前先调用
        
        if((_revents & EPOLLIN) || (_revents & EPOLLRDHUP) ||(_revents & EPOLLPRI))
        {
            // 如果是有数据可读、对端关闭写入、有带外数据的事件触发的话，则都属于是可读事件处理
            if(_read_callback)
                _read_callback();
            
            if(_arbitrary_callback)
                _arbitrary_callback(); // 不管任何事件，都调用的回调函数
        }

        // 下面的三个事件有可能会释放连接，所以只能处理一个，要用else if连接
        if(_revents & EPOLLOUT) 
        {
            if(_write_callback)
                _write_callback(); // 可读事件触发的处理
            
            if(_arbitrary_callback)
                _arbitrary_callback(); // 不管任何事件，都调用的回调函数
        }
        else if(_revents & EPOLLERR) 
        {
            if(_arbitrary_callback)
                _arbitrary_callback(); // 不管任何事件，都调用的回调函数
                
            if(_error_callback)
                _error_callback(); // 错误事件触发的处理
        }
        else if(_revents & EPOLLHUP) 
        {
            if(_arbitrary_callback)
                _arbitrary_callback(); // 不管任何事件，都调用的回调函数

            if(_close_callback)
                _close_callback(); // 关闭事件触发的处理
        }
    }
};

#endif