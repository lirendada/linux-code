#include "../server.hpp"

class EchoServer
{
private:
    TcpServer _server;
public:
    EchoServer(uint16_t port)
        : _server(port)
    {
        // 对服务器进行设置
        _server.set_nums_of_subthread(3);
        _server.enable_inactive_release(5);
        _server.set_connected_callback(std::bind(&EchoServer::connected_handle, this, std::placeholders::_1));
        _server.set_message_callback(std::bind(&EchoServer::message_handle, this, std::placeholders::_1, std::placeholders::_2));
        _server.set_closed_callback(std::bind(&EchoServer::closed_handle, this, std::placeholders::_1));
    }

    // 启动回显服务器接口
    void start() { _server.start_server(); }
private:
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
};