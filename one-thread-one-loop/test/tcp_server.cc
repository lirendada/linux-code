#include "../source/server.hpp"

void CloseEvent(Channel* channel)
{
    DLOG("close：%d", channel->get_fd());
    if(channel == nullptr || channel->get_fd() < 0)
        return;
    channel->clear_callback();
    channel->remove(); // 移除监控
    delete channel;
}

void ReadEvent(Channel* channel)
{
    // 这里读事件处理，我们就做简单的打印、启动可写事件监控即可
    int fd = channel->get_fd();
    char buffer[1024] = { 0 };
    int n = recv(fd, buffer, sizeof(buffer) - 1, 0);
    if(n > 0)
    {
        buffer[n] = 0;
        DLOG("接收到：%s", buffer);

        // 接收到数据之后，启动可写事件监控
        channel->enable_write();
    }
    else
        CloseEvent(channel); // 其实不应该释放，但是因为当前只是测试，所以需要关闭
}

void WriteEvent(Channel* channel)
{
    // 这里做个简单的发送即可
    int fd = channel->get_fd();
    const char* data = "lirendada 你好呀！";
    int n = send(fd, data, strlen(data), 0);
    if(n < 0)
    {
        return CloseEvent(channel); // 错误的话释放该对象
    }
    channel->disable_write(); // 然后关闭可写事件监控
}

void ErrorEvent(Channel* channel)
{
    CloseEvent(channel); // 错误的话释放该对象
}

void ArbitraryEvent(Channel* channel, EventLoop* loop, uint64_t timerid)
{
    // 刷新非活跃连接
    loop->refresh_timer(timerid);
}

void Acceptor(Channel* listen_channel, EventLoop* loop)
{
    // 获取新链接
    int newfd = accept(listen_channel->get_fd(), nullptr, nullptr);
    if(newfd < 0)
    {
        ELOG("accept error, the error is: ", strerror(errno));
        return;
    }

    // 设置新链接的回调函数
    uint64_t id = rand() % 10000;
    Channel* channel = new Channel(newfd, loop);
    channel->set_read_callback(std::bind(ReadEvent, channel));
    channel->set_write_callback(std::bind(WriteEvent, channel));
    channel->set_close_callback(std::bind(CloseEvent, channel));
    channel->set_error_callback(std::bind(ErrorEvent, channel));
    channel->set_arbitrary_callback(std::bind(ArbitraryEvent, channel, loop, id));

    // 添加定时任务，即对新连接进行过期删除操作
    loop->add_timer(id, 10, std::bind(CloseEvent, channel));
    
    // 启动新链接的可读事件监控
    channel->enable_read();
}

int main()
{
    srand(time(nullptr));

    // 创建服务器套接字
    Socket server;
    server.create_server(8080);

    // 创建一个EventLoop对象
    EventLoop loop;

    // 创建一个用于监听套接字的Channel对象，然后利用bind函数设置可读回调函数，并且启动可读监控
    Channel listen_channel(server.get_fd(), &loop);
    listen_channel.set_read_callback(std::bind(Acceptor, &listen_channel, &loop));
    listen_channel.enable_read();
    while(true)
    {
        loop.start();
    }
    server.Close();
    return 0;
}