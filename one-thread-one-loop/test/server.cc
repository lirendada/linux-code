#include "../source/server.hpp"

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
}

int main()
{
    // 创建tcpserver对象，进行设置后直接启动服务器即可
    TcpServer server(8080);
    server.set_nums_of_subthread(3);
    server.enable_inactive_release(5);
    server.set_connected_callback(connected_handle);
    server.set_message_callback(message_handle);
    server.set_closed_callback(closed_handle);

    server.start_server();
    return 0;
}