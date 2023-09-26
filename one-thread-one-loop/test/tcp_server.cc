#include "../source/server.hpp"

void CloseEvent(Channel* channel)
{
    std::cout << "close: " << channel->get_fd() << std::endl;
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
        std::cout << buffer << std::endl;

        // 接收到数据之后，启动可写事件监控
        channel->enable_write();
    }
    else
    {
        CloseEvent(channel); // 其实不应该释放，但是因为当前只是测试，所以需要关闭
    }
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

void ArbitraryEvent(Channel* channel)
{
    std::cout << "有一个事件发生，fd：" << channel->get_fd() << std::endl;
}

void Acceptor(Channel* listen_channel, Poller* poller)
{
    // 获取新链接
    int newfd = accept(listen_channel->get_fd(), nullptr, nullptr);
    if(newfd < 0)
    {
        ELOG("accept error, the error is: ", strerror(errno));
        return;
    }

    // 设置新链接的回调函数
    Channel* channel = new Channel(newfd, poller);
    channel->set_read_callback(std::bind(ReadEvent, channel));
    channel->set_write_callback(std::bind(WriteEvent, channel));
    channel->set_close_callback(std::bind(CloseEvent, channel));
    channel->set_error_callback(std::bind(ErrorEvent, channel));
    channel->set_arbitrary_callback(std::bind(ArbitraryEvent, channel));

    // 启动新链接的可读事件监控
    channel->enable_read();
}

int main()
{
    // 创建服务器套接字
    Socket server;
    server.create_server(8080);

    // 创建一个poller对象
    Poller poller;

    // 创建一个用于监听套接字的Channel对象，然后利用bind函数设置可读回调函数，并且启动可读监控
    Channel listen_channel(server.get_fd(), &poller);
    listen_channel.set_read_callback(std::bind(Acceptor, &listen_channel, &poller));
    listen_channel.enable_read();

    while(true)
    {
        // 开始监听事件，然后获取事件之后进行函数回调
        std::vector<Channel*> actives;
        poller.start_event(&actives);
        for(int i = 0; i < actives.size(); ++i)
        {
            actives[i]->handler();
        }
    }
    server.Close();
    return 0;
}