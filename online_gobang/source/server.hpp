#ifndef __MY_SERVER_H__
#define __MY_SERVER_H__
#include "db.hpp"
#include "online.hpp"
#include "util.hpp"
#include "session.hpp"
#include "matcher.hpp"
#include "room.hpp"

#define WEBROOT "./wwwroot/"

class gobang_server
{
private:
    std::string _webroot;        // 静态资源目录，默认为./wwwroot/
    wsserver_t _server;          // websocket服务器实例
    user_table _user_table;      // 数据库用户信息实例
    online_manager _online_user; // 在线用户管理实例
    session_manager _session;    // 会话管理实例
    room_manager _room;          // 房间管理实例
    match_manager _matcher;      // 匹配管理实例
public:
    // 构造函数，完成成员变量的初始化以及服务器的设置
    gobang_server(const std::string& host,
                  const std::string& user,
                  const std::string& passwd,
                  const std::string& dbname,
                  uint16_t port = 3306,
                  const std::string& webroot = WEBROOT)
        : _webroot(webroot)
        , _user_table(host, user, passwd, dbname, port)
        , _session(&_server)
        , _room(&_user_table, &_online_user)
        , _matcher(&_online_user, &_user_table, &_room)
    {
        // 设置日志等级
        _server.set_access_channels(websocketpp::log::alevel::none);
        // 初始化asio调度器和地址重用
        _server.init_asio();
        _server.set_reuse_addr(true);
        // 设置回调函数
        _server.set_http_handler(std::bind(&gobang_server::http_callback, this, std::placeholders::_1));
        _server.set_open_handler(std::bind(&gobang_server::open_callback, this, std::placeholders::_1));
        _server.set_close_handler(std::bind(&gobang_server::close_callback, this, std::placeholders::_1));
        _server.set_message_handler(std::bind(&gobang_server::message_callback, this, std::placeholders::_1, std::placeholders::_2));
    }

    // 启动服务器接口
    void start(uint16_t port)
    {
        _server.listen(port);   // 设置监听窗口
        _server.start_accept(); // 开始获取新连接
        _server.run();          // 启动服务器
    }

private:
    /*                 http请求处理函数                 */
    // 静态资源请求处理函数
    void static_resource_handler(wsserver_t::connection_ptr& conn)
    {
        // 1. 获取请求的uri资源路径
        websocketpp::http::parser::request req = conn->get_request();
        std::string uri = req.get_uri();

        // 2. 通过根目录和请求的uri组合成实际路径
        std::string path = _webroot + uri;

        // 3. 如果请求的是一个目录，那么就默认增加一个后缀login.html
        if(path.back() == '/')
            path += "login.html";

        // 4. 读取文件内容，并且判断是否读写成功
        std::string body;
        bool ret = file_util::read(path, body);
        if(ret == false)
        {
            // 读写失败则返回一个状态码为404的NOT FOUND页面
            body += "<html>";
            body += "<head>";
            body += "<meta charset='UTF-8'/>";
            body += "</head>";
            body += "<body>";
            body += "<h1> Not Found </h1>";
            body += "</body>";
            body += "</html>";
            conn->set_status(websocketpp::http::status_code::not_found);
            conn->set_body(body);
            return;
        }

        // 5. 读写成功后设置响应正文，也就是将静态资源传回去
        conn->set_status(websocketpp::http::status_code::ok);
        conn->set_body(body);
    }

    // 注册功能请求的处理函数
    void register_handler(wsserver_t::connection_ptr& conn)
    {
        // 1. 获取到请求正文 -- 为了获取用户名和密码
        std::string req_body = conn->get_request_body();

        // 2. 对正文进行反序列化，得到用户名和密码
        Json::Value body;
        bool ret = json_util::unserialize(req_body, body);
        if(ret == false)
        {
            // 反序列化失败的话要返回错误响应
            DLOG("反序列化注册信息失败");
            return http_response(conn, false, websocketpp::http::status_code::bad_request, "请求的正文格式错误");
        }

        // 3. 判断用户名和密码是否填写完整
        if(body["username"].isNull() || body["password"].isNull())
        {
            // 填写不完整的话要返回错误响应
            DLOG("用户名密码不完整");
            return http_response(conn, false, websocketpp::http::status_code::bad_request, "请输入用户名和密码");
        }

        // 4. 进行数据库的用户信息操作 -- 如果成功状态码返回200，失败返回400
        ret = _user_table.sign_up(body);
        if(ret == false)
        {
            // 写入用户信息错误的话要返回错误响应
            DLOG("向数据库插入数据失败");
            return http_response(conn, false, websocketpp::http::status_code::bad_request, "该用户名已经被占用!");
        }
        http_response(conn, true, websocketpp::http::status_code::ok, "用户注册成功");
    }

    // 登录功能请求的处理函数
    void login_handler(wsserver_t::connection_ptr& conn)
    {
        // 1. 获取请求正文，进行反序列化，得到用户名和密码
        std::string req_body = conn->get_request_body();
        Json::Value body;
        bool ret = json_util::unserialize(req_body, body);
        if(ret == false)
        {
            DLOG("反序列化登录信息失败");
            return http_response(conn, false, websocketpp::http::status_code::bad_request, "请求的正文格式错误");
        }

        // 2. 验证用户名和密码是否都填写
        if(body["username"].isNull() || body["password"].isNull())
        {
            DLOG("用户名密码不完整");
            return http_response(conn, false, websocketpp::http::status_code::bad_request, "请输入用户名和密码");
        }

        // 3. 进行数据库的用户信息验证，验证用户名和密码 -- 验证失败则返回400
        ret = _user_table.login(body);
        if(ret == false)
        {
            DLOG("用户不存在或者密码错误");
            return http_response(conn, false, websocketpp::http::status_code::bad_request, "请输入正确的用户名和密码");
        }

        // 4. 如果验证成功，则给客户端创建session，并且设置过期时间
        session_ptr sp = _session.add_session(body["id"].asUInt64(), LOGIN);
        if(sp.get() == nullptr)
        {
            // 创建会话失败的话返回的状态码是500
            DLOG("创建会话失败");
            return http_response(conn, false, websocketpp::http::status_code::internal_server_error, "创建会话失败");
        }
        _session.set_session_expire_time(sp->getSessionID(), SESSION_EXPIRE_TIME);

        // 5. 设置响应头部：Set-Cookie，将sessionID通过cookie响应返回
        conn->append_header("Set-Cookie", "SSID=" + std::to_string(sp->getSessionID()));
        http_response(conn, true, websocketpp::http::status_code::ok, "登录成功");
    }

    // 获取个人信息功能请求的处理函数
    void information_handler(wsserver_t::connection_ptr& conn)
    {

    }
private:
    void http_callback(websocketpp::connection_hdl hdl)
    {
        // 1. 获取请求的方法和uri，用于区分不同的请求类型
        wsserver_t::connection_ptr conn = _server.get_con_from_hdl(hdl);
        websocketpp::http::parser::request req = conn->get_request();
        std::string method = req.get_method();
        std::string uri = req.get_uri();
        
        // 2. 根据不同的请求类型调用对应的处理函数
        if(method == "POST" && uri == "/reg")          // 注册请求
            return register_handler(conn);
        else if(method == "POST" && uri == "/login")   // 登录请求
            return login_handler(conn);
        else if(method == "GET" && uri == "/userinfo") // 获取用户信息请求
            return information_handler(conn);
        else
            return static_resource_handler(conn);       // 剩下的都认为是静态资源请求
    }

    void open_callback(websocketpp::connection_hdl hdl)
    {}

    void close_callback(websocketpp::connection_hdl hdl)
    {}

    void message_callback(websocketpp::connection_hdl hdl, wsserver_t::message_ptr msgptr)
    {}
private:
    // http响应处理通用函数
    void http_response(wsserver_t::connection_ptr& conn, 
                       bool result, 
                       websocketpp::http::status_code::value status,
                       const std::string& reason)
    {
        // 1. 填写响应信息
        Json::Value response;
        response["result"] = result;
        response["reason"] = reason;
        
        // 2. 进行序列化
        std::string response_body;
        json_util::serialize(response, response_body);

        // 3. 设置http响应，类型为application/json
        conn->set_body(response_body);
        conn->set_status(status);
        conn->append_header("Content-Type", "application/json");
    }
};

#endif