#include <iostream>
#include <string>
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>

using wsserver_t = websocketpp::server<websocketpp::config::asio>;

void open_callback(wsserver_t* server, websocketpp::connection_hdl hdl)
{
    std::cout << "websocket握手成功！" << std::endl;
}

void close_callback(wsserver_t* server, websocketpp::connection_hdl hdl)
{
    std::cout << "websocket断开连接！" << std::endl;
}

// 任务：打印请求内容，并且设置响应内容
void http_callback(wsserver_t* server, websocketpp::connection_hdl hdl)
{
    // 首先将一些请求内容打印到终端
    wsserver_t::connection_ptr conn_ptr = server->get_con_from_hdl(hdl); // 获取connection_ptr
    std::cout << "body: " << conn_ptr->get_request_body() << std::endl;  // 打印请求正文

    websocketpp::http::parser::request req = conn_ptr->get_request();    // 获取http请求对象
    std::cout << "method: " << req.get_method() << std::endl;            // 打印请求方法
    std::cout << "uri: " << req.get_uri() << std::endl;                  // 打印请求路径资源
    std::cout << "version: " << req.get_version() << std::endl;          // 打印请求版本

    // 然后再响应资源
    std::string body = "<html><body><h1>Hello Liren!</h1></body></html>"; // 写一个简单的html页面格式
    conn_ptr->set_body(body);                                             // 设置响应正文
    conn_ptr->append_header("Content-Type", "text/html");                 // 设置响应头部
    conn_ptr->set_status(websocketpp::http::status_code::ok);             // 设置响应状态码
}

// 任务：收到一个消息进行打印，然后进行响应
void message_callback(wsserver_t* server, websocketpp::connection_hdl hdl, wsserver_t::message_ptr msgptr)
{
    // 打印获取的消息
    wsserver_t::connection_ptr conn_ptr = server->get_con_from_hdl(hdl); // 获取connection_ptr
    std::cout << "message: " << msgptr->get_payload() << std::endl;      // 打印消息

    // 响应
    std::string callback = "server say: client say " + msgptr->get_payload(); // 创建响应内容
    conn_ptr->send(callback, websocketpp::frame::opcode::text);               // 发送响应
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