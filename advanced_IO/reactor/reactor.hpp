#pragma once 
#include <unordered_map>
#include <functional>
#include "epoller.hpp"
#include "err.hpp"
#include "log.hpp"
#include "sock.hpp"
#include "connection.hpp"
#include "protocol.hpp"

class connection;
using func_t = std::function<void(connection*)>;

static const int default_port = 8080; // 默认端口
static const int event_size = 100;    // 就绪事件数组的大小
static const int epoll_size = 128;    // 内核中epoll_event的个数
static const int buffer_size = 1024;  // 收发缓冲区的大小

class reactor
{
private:
    int _port;                                      // 服务器端口号
    sock _sock;                                     // 监听套接字对象
    int _epfd;                                      // epoll模型的文件描述符
    struct epoll_event* _events;                    // 就绪事件数组
    epoller _epoller;                               // epoller对象
    unordered_map<int, connection*> _connect_table; // connection对象的集合
    func_t _service;                                // 业务处理类型
public:
    reactor(func_t service, int port = default_port)
        : _port(port), _epfd(default_num), _events(nullptr), _service(service)
    {
        // 1. 完成套接字初始化
        _sock.Socket();
        _sock.Bind(_port);
        _sock.Listen();

        // 2. 创建epoll模型
        _epfd = _epoller.create(epoll_size);

        // 3. 将监听套接字交给epoll模型管理，并且添加到哈希表中管理
        _epoller.control(_sock.GetFD(), EPOLL_CTL_ADD, EPOLLIN | EPOLLET);
        addConnection(_sock.GetFD(), std::bind(&reactor::Accepter, this, std::placeholders::_1), nullptr, nullptr);

        // 4. 开辟就绪事件数组
        _events = new struct epoll_event[event_size];
        if(_events == nullptr)
        {
            logMessage(Level::ERROR, "new epoll_event error, errno: %d, string_err: %s", errno, strerror(errno));
            exit(NEW_EVENTS_ERR);
        }
        logMessage(Level::NORMAL, "new epoll_event success");
    }

    ~reactor()
    {
        if(_epfd != -1)
            close(_epfd);
        if(_events != nullptr)
            delete[] _events;
    }    

    // 运行函数，监听epoll中的就绪事件，根据类型派发给不同的处理函数
    void run()
    {
        while(true)
        {
            int n = _epoller.wait(_events, event_size);
            if(n == -1)
            {
                logMessage(Level::ERROR, "epoll wait error, errno: %d, string_err: %s", errno, strerror(errno));
                exit(EPOLL_WAIT_ERR);
            }
            else if(n == 0)
            {
                logMessage(Level::NORMAL, "epoll wait timeout...");
            }
            else
            {
                // 遍历所有就绪事件
                for(int i = 0; i < n; ++i)
                {
                    int fd = _events[i].data.fd;
                    uint32_t event = _events[i].events;

                    // 如果是错误事件，则变成EPOLLIN和EPOLLOUT事件去解决，其处理函数中会转化为异常处理
                    if((event & EPOLLERR) || (event & EPOLLHUP)) 
                        event |= (EPOLLIN & EPOLLOUT);

                    if(event & EPOLLIN)
                        _connect_table[fd]->_receiver(_connect_table[fd]);
                    if(event & EPOLLOUT)
                        _connect_table[fd]->_sender(_connect_table[fd]);
                }
            }
        }
    }
    
    void addConnection(int fd, func_t receiver, func_t sender, func_t exception)
    {
        // 1. 创建一个connection对象，然后初始化
        connection* conn = new connection(fd, this, receiver, sender, exception);
        if(conn == nullptr)
        {
            logMessage(Level::ERROR, "new connection error, errno: %d, string_err: %s", errno, strerror(errno));
            exit(NEW_CONNECTION_ERR);
        }

        // 2. 将其添加到哈希表中维护
        _connect_table[fd] = conn;
        logMessage(Level::DEBUG, "addConnection: %d in unordered_map", conn->_fd);
    }

    void EnableReadWrite(connection *conn, bool readable, bool writeable)
    {
        uint32_t event = (readable ? EPOLLIN : 0) | (writeable ? EPOLLOUT : 0) | EPOLLET;
        _epoller.control(conn->_fd, event, EPOLL_CTL_MOD);
    }
private:
    void Accepter(connection* conn)
    {
        // 必须循环读，直到内容都被读取上来为止
        while(true)
        {
            // 获取新链接
            std::string clientip;
            std::uint16_t clientport;
            int err;
            int newfd = _sock.Accept(&clientip, &clientport, &err);
            if(newfd == -1)
            {
                if(err == EAGAIN || err == EWOULDBLOCK) 
                {
                    logMessage(Level::NORMAL, "accept数据已经读取完整，退出！");
                    break;
                }
                else if(err == EINTR)
                {
                    logMessage(Level::NORMAL, "被中断了，要继续读取！");
                    continue;
                }
                else
                {
                    logMessage(Level::ERROR, "accept数据已经读取完整，退出！");
                    break;
                }
            }
            else
            {
                // 将新链接交给epoll管理，并且添加到哈希表中管理
                _epoller.control(newfd, EPOLL_CTL_ADD, EPOLLIN | EPOLLET);
                addConnection(
                    newfd, 
                    std::bind(&reactor::Receiver, this, std::placeholders::_1),
                    std::bind(&reactor::Sender, this, std::placeholders::_1),
                    std::bind(&reactor::Excepter, this, std::placeholders::_1)
                );
                
                logMessage(Level::DEBUG, "get a new link, info: [%s:%d]", clientip.c_str(), clientport);
            }
        }
    }
    
    void Receiver(connection* conn)
    {
        // 必须循环读，直到内容都被读取上来为止
        char buffer[buffer_size];
        while(true)
        {
            ssize_t n = recv(conn->_fd, buffer, sizeof(buffer) - 1, 0);
            if(n > 0)
            {
                buffer[n] = 0;
                conn->_inbuffer += buffer; // 进行尾插到缓冲区
                logMessage(Level::DEBUG, "%s", conn->_inbuffer.c_str());

                // 将读到缓冲区的数据交给业务处理函数去拆解
                _service(conn);
            }
            else if(n == 0)
            {
                // 请求断开连接，则直接交给异常处理
                if (conn->_excepter)
                {
                    conn->_excepter(conn);
                    return;
                }
            }
            else
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK) // 数据读完了直接break
                    return;
                else if (errno == EINTR) // 读取中断，则继续读取
                    continue;
                else
                {
                    if (conn->_excepter) // 异常的话交给异常处理
                    {
                        conn->_excepter(conn);
                        return;
                    }
                }
            }
        }
    }

    void Sender(connection* conn)
    {
        // 必须循环读，直到内容都被读取上来为止
        while(true)
        {
            ssize_t n = send(conn->_fd, conn->_outbuffer.c_str(), conn->_outbuffer.size(), 0);
            if(n > 0)
            {
                if(conn->_outbuffer.empty())
                    break;
                else
                    conn->_outbuffer.erase(0, n);
            }
            else
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK) // 缓冲区满了直接break
                    break;
                else if (errno == EINTR) // 发送中断，则继续读取
                    continue;
                else
                {
                    if (conn->_excepter) // 异常的话交给异常处理
                    {
                        conn->_excepter(conn);
                        return;
                    }
                }
            }
        }
        // 如果没有发送完毕，需要对对应的sock开启对写事件的关系， 如果发完了，我们要关闭对写事件的关心！
        if(!conn->_outbuffer.empty())
            conn->_rp->EnableReadWrite(conn, true, true);
        else
            conn->_rp->EnableReadWrite(conn, true, false);
    }

    void Excepter(connection* conn)
    {
        logMessage(DEBUG, "关闭%d 文件描述符的所有的资源", conn->_fd);
        _epoller.control(conn->_fd, 0, EPOLL_CTL_DEL);
        conn->Close();
        _connect_table.erase(conn->_fd);
        delete conn;
    }
};