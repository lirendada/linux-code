#include <iostream>
#include <string>
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>

using wsserver_t = websocketpp::server<websocketpp::config::asio>;

void open_callback(wsserver_t* server, websocketpp::connection_hdl hdl)
{

}

void close_callback(wsserver_t* server, websocketpp::connection_hdl hdl)
{
    
}

void http_callback(wsserver_t* server, websocketpp::connection_hdl hdl)
{
    // 给客户端返回一个页面
    wsserver_t::connection_ptr connection_ptr = server->get_con_from_hdl(hdl);
    
}

void message_callback(wsserver_t* server, websocketpp::connection_hdl hdl, wsserver_t::message_ptr msghdl)
{
    
}

int main()
{
    // 1.实例化server对象
    wsserver_t server;

    // 2.设置日志等级
    server.set_access_channels(websocketpp::log::alevel::none);

    // 3.初始化asio调度器和地址重用
    server.init_asio();
    server.set_reuse_addr(true);

    // 4.设置回调函数
    server.set_open_handler(std::bind(open_callback, &server, std::placeholders::_1));
    server.set_close_handler(std::bind(close_callback, &server, std::placeholders::_1));
    server.set_message_handler(std::bind(message_callback, &server, std::placeholders::_1, std::placeholders::_2));
    server.set_http_handler(std::bind(http_callback, &server, std::placeholders::_1));

    // 5.设置监听窗口
    server.listen(8080);

    // 6.开始获取新连接
    server.start_accept();

    // 7.启动服务器
    server.run();

    return 0;
}