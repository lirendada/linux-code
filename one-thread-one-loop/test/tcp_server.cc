#include "../source/server.hpp"

uint64_t id = 1; // 连接id
std::unordered_map<uint64_t, ConnectionPtr> connections; // 连接管理表

void connected_handle(const ConnectionPtr& cptr)
{
    // 这里的连接建立处理，我们就简单的打印哪个连接建立即可
    DLOG("new connection: %p，the id is：%d", cptr.get(), cptr->get_connection_id());
}

void message_handle(const ConnectionPtr& cptr, Buffer* buf)
{
    // 这里的消息事件处理，我们就做简单的打印以及回响即可
    DLOG("接收到：%s", buf->start_of_read());
    buf->push_reader_back(buf->get_sizeof_read());

    std::string str = "lirendada 你好啊！";
    cptr->send_data(str.c_str(), str.size());
}

void closed_handle(const ConnectionPtr& cptr)
{
    // 就是将连接管理表中的该连接去掉
    DLOG("delete connection: %p，the id is：%d", cptr.get(), cptr->get_connection_id());
    connections.erase(cptr->get_connection_id());
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

    // 用Connection包装该新链接，并且设置回调函数
    ConnectionPtr cptr(new Connection(loop, id, newfd));
    cptr->set_connected_callback(std::bind(connected_handle, std::placeholders::_1));
    cptr->set_message_callback(std::bind(message_handle, std::placeholders::_1, std::placeholders::_2));
    cptr->set_server_closed_callback(std::bind(closed_handle, std::placeholders::_1)); // 注意这里是服务器模块的关闭回调，也就是去掉与该连接的联系

    // 启动非活跃销毁功能，并将连接设置为建立完成状态
    cptr->enable_inactive_release(3);
    cptr->connecting_to_connceted();

    // 最后别忘了添加到服务器的连接管理表中
    connections[id++] = cptr;
}

int main()
{
    // 创建服务器套接字
    Socket server;
    bool ret = server.create_server(8080);
    if(ret == false)
        return -1;

    // 创建一个EventLoop对象
    EventLoop loop;

    // 创建一个用于监听套接字的Channel对象，然后利用bind函数设置可读回调函数，并且启动可读监控
    Channel listen_channel(server.get_fd(), &loop);
    listen_channel.set_read_callback(std::bind(Acceptor, &listen_channel, &loop));
    listen_channel.enable_read();
    loop.start();
    return 0;
}