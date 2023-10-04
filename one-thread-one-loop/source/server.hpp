#ifndef __MY_SERVER_H__
#define __MY_SERVER_H__
#include <iostream>
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/eventfd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/timerfd.h>
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

    int get_fd() { return _sockfd; }

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
        DLOG("create_server success!");
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

class EventLoop;
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

    EventLoop* _eventpoller;
public:
    Channel(int fd, EventLoop* eventpoller) 
        : _fd(fd), _events(0), _revents(0), _eventpoller(eventpoller)
    {}

    ~Channel() 
    {
        close(_fd); // 记得要释放文件描述符 
    }

    int get_fd() { return _fd; }                               // 获取文件描述符
    uint32_t get_events() { return _events; }                  // 获取当前监控的事件
    void set_revents(uint32_t revents) { _revents = revents; } // 设置实际就绪的事件

    // 设置对应触发事件的回调函数
    void set_read_callback(const eventcallback_t& cb) { _read_callback = cb; }
    void set_write_callback(const eventcallback_t& cb) { _write_callback = cb; }
    void set_error_callback(const eventcallback_t& cb) { _error_callback = cb; }
    void set_close_callback(const eventcallback_t& cb) { _close_callback = cb; }
    void set_arbitrary_callback(const eventcallback_t& cb) { _arbitrary_callback = cb; }

    bool is_read_able()  { return (_events & EPOLLIN); }    // 当前是否监控了可读
    bool is_write_able() { return (_events & EPOLLOUT); }  // 当前是否监控了可写

    // 启动读事件监控
    void enable_read() { _events |= EPOLLIN; update(); }

     // 启动写事件监控
    void enable_write() { _events |= EPOLLOUT; update(); }

    // 关闭读事件监控
    void disable_read() { _events &= (~EPOLLIN); update(); }

    // 关闭写事件监控
    void disable_write() { _events &= (~EPOLLOUT); update(); }

    // 关闭所有事件监控
    void disable_all() { _events = 0; update(); } 

    // 清除所有的回调函数
    void clear_callback() 
    {
        _read_callback = _write_callback = _error_callback = _close_callback = _arbitrary_callback = nullptr;
    }

    // 事件总处理函数。一旦触发了事件，就调用这个函数，而触发了什么事件如何处理由连接管理者决定
    void handler()  
    {
        // 下面因为错误和关闭事件触发的时候会释放连接，此时就不能再调用_arbitrary_callback了，所以需要提前先调用
        if(_arbitrary_callback)
            _arbitrary_callback(); // 不管任何事件，都调用的回调函数

        if((_revents & EPOLLIN) || (_revents & EPOLLRDHUP) ||(_revents & EPOLLPRI))
        {
            // 如果是有数据可读、对端关闭写入、有带外数据的事件触发的话，则都属于是可读事件处理
            //DLOG("read_callback");
            if(_read_callback)
                _read_callback();
        }

        // 下面的三个事件有可能会释放连接，所以只能处理一个，要用else if连接
        if(_revents & EPOLLOUT) 
        {
            //DLOG("write_callback");
            if(_write_callback)
                _write_callback(); // 可读事件触发的处理
        }
        else if(_revents & EPOLLERR) 
        {
            //DLOG("error_callback");
            if(_error_callback)
                _error_callback(); // 错误事件触发的处理
        }
        else if(_revents & EPOLLHUP) 
        {
            //DLOG("close_callback");
            if(_close_callback)
                _close_callback(); // 关闭事件触发的处理
        }
    }

    // 添加或者修改事件监控
    void update();

    // 移除事件监控
    void remove();
};

const static int MAX_EPOLL_EVENTS = 1024;

class Poller
{
private:
    int _epollfd;                                
    struct epoll_event _events[MAX_EPOLL_EVENTS]; // 存放活跃连接的数组
    std::unordered_map<int, Channel*> _channels;  // 存放文件描述符与其对应的事件管理对象的哈希表
public:
    Poller() 
    {
        _epollfd = epoll_create(MAX_EPOLL_EVENTS);
        if(_epollfd == -1)
        {
            ELOG("epoll create error!!");
            abort();
        }
        DLOG("epoll create succuess, epollfd is: %d", _epollfd);
    }

    ~Poller() 
    {
        if(_epollfd != -1)
            close(_epollfd);
    }

    // 添加或修改监控事件（只是一个封装）
    void update_event(Channel* channel)
    {
        // 如果事件存在的话则属于修改事件，不存在的话属于添加事件
        bool ret = has_channel(channel);

        if(ret == false)
        {
            _channels[channel->get_fd()] = channel;
            control(channel, EPOLL_CTL_ADD); // 交给辅助函数去完成
        }
        else
            control(channel, EPOLL_CTL_MOD);
    }

    // 移除监控
    void remove_event(Channel* channel)
    {
        // 移除监控包括两个步骤：
        //   1. 从epoll模型中移除
        control(channel, EPOLL_CTL_DEL);

        //   2. 从哈希表中去除关系
        auto it = _channels.find(channel->get_fd());
        if(it != _channels.end())
            _channels.erase(it);
    }

    // 启动监控，返回活跃连接
    void start_event(std::vector<Channel*>* active)
    {
        // 等待事件就绪，我们这里设为阻塞式等待
        int n = epoll_wait(_epollfd, _events, MAX_EPOLL_EVENTS, -1);  
        if(n == -1)
        {
            if(errno == EINTR) // 如果是被中断则不代表等待失败
                return;

            ELOG("epoll_wait error:%s\n", strerror(errno));
            abort();
        }

        // 将获取到的事件设置到对应的Channel对象中，并且尾插到active中
        for(int i = 0; i < n; ++i)
        {
            auto it = _channels.find(_events[i].data.fd);
            assert(it != _channels.end()); // 认为是一定能找到的，找不到说明是程序问题，直接退出

            it->second->set_revents(_events[i].events); // 设置实际就绪的事件
            active->push_back(it->second);
        }
    }
private:
    // 对epoll的实际操作接口
    void control(Channel* channel, int option)
    {
        struct epoll_event ev;
        ev.data.fd = channel->get_fd();
        ev.events = channel->get_events();
        int ret = epoll_ctl(_epollfd, option, channel->get_fd(), &ev);
        if(ret == -1)
            ELOG("epoll_ctl error!!");
    }

    // 判断一个事件管理对象Channel是否已经添加了事件监控
    bool has_channel(Channel* channel)
    {
        auto it = _channels.find(channel->get_fd());
        if(it == _channels.end())
            return false;
        return true;
    }
};

using func_t = std::function<void()>;        // 超时任务的函数类型，由使用者传入
using remove_t = std::function<void()>;      // 用于释放weak_ptr的函数类型，由TimerWheel传入
// 定时任务类，封装一个定时任务
class TimerTask
{
private:
    uint64_t _id;       // 当前超时任务类的ID
    uint32_t _timeout;  // 超时时间
    func_t _task;       // 超时任务
    remove_t _remove;   // 释放TimerWheel中的weak_ptr
    bool _cancel;       // 为true表示要取消任务，为false表示正常执行任务
public:
    TimerTask(uint64_t id, uint32_t timeout, const func_t& task) 
        : _id(id)
        , _timeout(timeout)
        , _task(task)
        , _cancel(false) 
    {}

    ~TimerTask()
    {
        // 如果没有取消任务，才执行释放函数
        if(_cancel == false)
        {
            // 析构函数进行超时任务以及weak_ptr释放函数的执行
            _task();
            _remove();
        }
    }

    uint64_t get_id() { return _id; }
    uint32_t get_timeout() { return _timeout; }
    void set_remove(const remove_t& remove) { _remove = remove; }
    void set_cancel() { _cancel = true; }
};

// 时间轮与timerfd的整合类
class TimerWheel
{
    using shared_t = std::shared_ptr<TimerTask>;
    using weak_t = std::weak_ptr<TimerTask>;
private:
    int _tick;                                     // 当前的时间轮秒数，每一秒就往后走一步
    int _capacity;                                 // 时间轮数组大小，即时间轮的周期
    std::vector<std::vector<shared_t>> _wheel;     // 时间轮数组
    std::unordered_map<uint64_t, weak_t> _table;   // 保存所有定时任务对象的weak_ptr，这样才能在不影响shared_ptr计数器的同时，获取其shared_ptr

    EventLoop* _loop; // 为了初始化_timer_channel和找到当前TimerWheel对应的EventLoop

    int _timerfd;                            // 定时器描述符
    std::unique_ptr<Channel> _timer_channel; // 对上面的定时器描述符进行事件管理
public:
    TimerWheel(EventLoop* loop) 
        : _tick(0)
        , _capacity(60)
        , _wheel(_capacity)
        , _loop(loop)
        , _timerfd(create_timerfd())
        , _timer_channel(new Channel(_timerfd, _loop))    
    {
        // 进行定时器的可读事件设置，当触发可读事件的时候，即超时之后，则进行定时任务的删除
        _timer_channel->set_read_callback(std::bind(&TimerWheel::timer_read, this));
        _timer_channel->enable_read();
    }

    /* 定时器中有个_table成员，定时器信息的操作有可能在多线程中进行，因此需要考虑线程安全问题 */
    /* 如果不想加锁，那就把对定期的所有操作，都放到一个线程中进行 */
    // 将添加超时任务操作添加到对应EventLoop的任务队列中
    void add_timertask_in_thread(uint64_t id, uint32_t timeout, const func_t& task);

    // 将刷新超时任务操作添加到对应EventLoop的任务队列中
    void refresh_timertask_in_thread(uint64_t id);

    // 将删除超时任务操作添加到对应EventLoop的任务队列中
    void cancel_timertask_in_thread(uint64_t id);

    // 判断是否存在定时任务（存在线程安全问题，只能在一个线程中使用）
    bool has_timertask(uint64_t id)
    {
        auto it = _table.find(id);
        if(it == _table.end())
            return false;
        return true;
    }
private:
    static int create_timerfd()
    {
        // 1. 创建定时器描述符
        int timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
        if(timerfd == -1)
        {
            ELOG("timerfd_create error");
            abort();
        }

        // 设置定时器
        struct itimerspec newtimer;
        newtimer.it_value.tv_sec = 1;    // 设置第一次超时的时间
        newtimer.it_value.tv_nsec = 0;
        newtimer.it_interval.tv_sec = 1; // 设置第一次超时后每次的超时间隔时间
        newtimer.it_interval.tv_nsec = 0;
        timerfd_settime(timerfd, 0, &newtimer, nullptr);

        DLOG("create_timerfd success, the timerfd is %d", timerfd);
        return timerfd;
    }

    // 定时器超时之后的处理函数
    void timer_read()
    {
        // 1. 读取计数器内容，即清空计数器
        // 有可能因为其他描述符的事件处理花费事件比较长，然后在处理定时器描述符事件的时候，有可能就已经超时了很多次
        // read读取到的数据times就是从上一次read之后超时的次数
        uint64_t times;
        int ret = read(_timerfd, &times, 8);
        if (ret < 0) {
            ELOG("READ TIMEFD FAILED!");
            abort();
        }

        // 2. 进行定时任务的删除
        for(int i = 0; i < times; ++i)
            run_timer();
    }

    // 时间运行函数
    void run_timer()
    {
        // 一秒钟走一步，每次将到达的位置处的shared_ptr进行清空，如果是最后一次任务的话会自动调用其析构函数进行释放
        _tick = (_tick + 1) % _capacity;
        _wheel[_tick].clear();
    }

    // 在哈希表中去除并且释放weak_ptr
    void remove_timer(uint64_t id)
    {
        // 先判断在不在哈希表中
        if(!has_timertask(id))
            return;

        _table.erase(id);
    }

    // 添加定时任务
    void add_timertask(uint64_t id, uint32_t timeout, const func_t& task)
    {
        // 1. 创建一个定时任务，由智能指针管理
        shared_t newtask(new TimerTask(id, timeout, task));
        if(newtask.get() == nullptr)
            return;
        
        // 2. 设置释放函数
        newtask->set_remove(std::bind(&TimerWheel::remove_timer, this, id));

        // 3. 向时间轮数组中添加定时任务
        int pos = (_tick + timeout) % _capacity; // 注意需要取模，防止越界
        _wheel[pos].push_back(newtask);

        // 4. 将定时任务交给哈希表管理，记得要使用weak_ptr才不会导致计数增加
        _table[id] = weak_t(newtask);
    }

    // 刷新定时任务
    void refresh_timertask(uint64_t id)
    {
        // 1. 首先通过哈希表找到保存的超时任务的weak_ptr
        auto it = _table.find(id);
        if(it == _table.end())
            return;
        
        // 2. 通过weak_ptr构造一个shared_ptr出来
        shared_t refresh_task(it->second.lock());

        // 3. 将刷新任务添加到时间轮数组中
        int pos = (_tick + refresh_task->get_timeout()) % _capacity; // 注意需要取模，防止越界
        _wheel[pos].push_back(refresh_task);
    }

    // 取消定时任务
    void cancel_timertask(uint64_t id)
    {
        // 先判断在不在哈希表中
        auto it = _table.find(id);
        if(it == _table.end())
            return;

        // 先拿到shared_ptr，再通过其取消任务
        shared_t st(it->second.lock()); 
        if(st.get() != nullptr)
            st->set_cancel();   
    }
};

using functor = std::function<void()>;
class EventLoop
{
private:
    std::thread::id _tid; // EventLoop对应的线程ID

    int _eventfd;                           // 用于唤醒线程等待事件就绪时候导致的阻塞
    std::unique_ptr<Channel> _eventchannel; // 用Channel对象维护上面的_eventfd事件

    Poller _poller; // 事件监控管理对象
    TimerWheel _tw; // 定时任务管理对象

    std::vector<functor> _tasks; // 任务队列（实际是一个数组，方便后面执行操作，减少队列的加锁消耗！）
    std::mutex _mtx;             // 互斥锁，保护任务队列操作
public:
    EventLoop()
        : _tid(std::this_thread::get_id())
        , _eventfd(create_eventfd())
        , _eventchannel(new Channel(_eventfd, this))
        , _tw(this)
    {
        // 启动eventfd的可读事件监控（启动可写事件监控无意义）
        _eventchannel->set_read_callback(std::bind(&EventLoop::event_read, this));
        _eventchannel->enable_read();
    }

    // 启动监控、就绪处理、执行任务总函数
    void start()
    {
        // 1. 启动监控
        std::vector<Channel*> actives;
        _poller.start_event(&actives);

        // 2. 处理就绪事件
        for(int i = 0; i < actives.size(); ++i)
            actives[i]->handler();
        
        // 3. 执行任务队列中的任务
        run_all_tasks();
    }   

    // 判断将要执行的任务是否处于当前线程中，如果是则直接执行，否则入队列
    void run_in_thread(const functor& callback)
    {
        if(is_in_thread())
            callback();
        else
            push(callback);
    }

    // 将任务入队列
    void push(const functor& callback)
    {
        {
            // 入队列要进行加锁
            std::unique_lock<std::mutex> lock(_mtx);
            _tasks.push_back(callback);
        }

        // 唤醒有可能因为没有事件就绪，而导致的epoll阻塞（其实很简单，就是给eventfd写入一条数据就能唤醒，因为触发了可读事件！）
        wakeup_eventfd();
    }     

    // 添加/修改事件监控
    void update_event(Channel* channel) { return _poller.update_event(channel); }     

    // 移除事件监控    
    void remove_event(Channel* channel) { return _poller.remove_event(channel); }         

    // 添加定时任务
    void add_timer(uint64_t id, uint32_t timeout, const func_t& task) { _tw.add_timertask_in_thread(id, timeout, task); }

    // 刷新定时任务
    void refresh_timer(uint64_t id) { _tw.refresh_timertask_in_thread(id); }

    // 删除定时任务
    void cancel_timer(uint64_t id) { _tw.cancel_timertask_in_thread(id); }

    // 判断是否存在定时任务
    bool has_timer(uint64_t id) { return _tw.has_timertask(id); }
public:    
    // 用于执行任务队列中的任务，该函数不给外界使用
    void run_all_tasks()
    {
        // 开辟一个空的临时数组，将其与任务队列中的数据进行交换，任务队列就变空了
        std::vector<functor> tmp;

        {
            // 交换过程要进行加锁
            std::unique_lock<std::mutex> lock(_mtx);
            _tasks.swap(tmp);
        }

        // 剩下的执行就交给临时数组即可，不需要考虑加锁问题
        for(int i = 0; i < tmp.size(); ++i)
            tmp[i]();
    }
private:
    static int create_eventfd()
    {
        int efd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
        if(efd < 0)
        {
            ELOG("create_eventfd error, 原因：%s", strerror(errno));
            abort();
        }
        DLOG("create_eventfd success, the eventfd is %d", efd);
        return efd;
    }
    
    // 判断当前线程是否是EventLoop对应的线程
    bool is_in_thread() { return _tid == std::this_thread::get_id(); }   

    // eventfd的可读事件就绪处理函数
    void event_read()
    {
        // 只需要做简单的读取，将内核计数器置零即可！
        uint64_t val = 0;
        int ret = read(_eventfd, &val, sizeof(val));
        if(ret < 0)
        {
            if(errno == EINTR || errno == EAGAIN) // 如果被打断或者缓冲区为空的话，不算是错误
                return;
            ELOG("read eventfd fail!");
            abort();
        } 
    }

    // 唤醒eventfd
    void wakeup_eventfd()
    {
        // 其实很简单，就是给eventfd写入一条数据就能唤醒，因为触发了可读事件！
        uint64_t val = 1;
        int ret = write(_eventfd, &val, sizeof(val));
        if(ret < 0)
        {
            if(errno == EINTR || errno == EAGAIN) // 如果被打断或者缓冲区为空的话，不算是错误
                return;
            ELOG("read eventfd fail!");
            abort();
        } 
    }
};

void Channel::update() { _eventpoller->update_event(this); }
void Channel::remove() { _eventpoller->remove_event(this); }

void TimerWheel::add_timertask_in_thread(uint64_t id, uint32_t timeout, const func_t& task) { _loop->run_in_thread(std::bind(&TimerWheel::add_timertask, this, id, timeout, task)); }
void TimerWheel::refresh_timertask_in_thread(uint64_t id) { _loop->run_in_thread(std::bind(&TimerWheel::refresh_timertask, this, id)); }
void TimerWheel::cancel_timertask_in_thread(uint64_t id) { _loop->run_in_thread(std::bind(&TimerWheel::cancel_timertask, this, id)); }

#endif