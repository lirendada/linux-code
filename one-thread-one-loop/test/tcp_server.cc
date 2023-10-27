#include "../source/server.hpp"

uint64_t id = 1; // 连接id
std::unordered_map<uint64_t, ConnectionPtr> connections; // 连接管理表

EventLoop server_loop;
LoopThreadPool threadpool(&server_loop);

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

void acceptor_callback(int sockfd)
{
    DLOG("我是服务器主线程，用于监听新连接！");

    // 用Connection包装该新链接，并且设置回调函数，其中新连接的EventLoop由线程池模块提供
    ConnectionPtr cptr(new Connection(threadpool.allocate_thread(), id, sockfd));

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
    // 初始化一下线程池管理模块
    threadpool.set_nums_of_subthread(3);
    threadpool.initialize();

    // 创建监听套接字，然后利用bind函数设置获取新连接之后的回调函数，并且启动可读监控
    Acceptor acceptor(&server_loop, 8080);
    acceptor.set_accept_callback(std::bind(acceptor_callback, std::placeholders::_1));
    acceptor.start_listen();

    // 启动事件监控
    server_loop.start();
    return 0;
}