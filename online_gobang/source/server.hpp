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
        else if(method == "GET" && uri == "/info") // 获取用户信息请求
            return information_handler(conn);
        else
            return static_resource_handler(conn);       // 剩下的都认为是静态资源请求
    }

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
            DLOG("静态资源请求异常,%s, 大小: %d", path.c_str(), body.size());
            // 读写失败则返回一个状态码为404的NOT FOUND页面
            body.clear();
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
        DLOG("静态资源请求成功, %s, 大小: %d", path.c_str(), body.size());
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
        // 1. 获取请求信息中的cookie信息
        std::string cookie_str = conn->get_request_header("Cookie");
        if(cookie_str.empty())
        {
            // 如果没有cookie信息，则返回错误，让客户端重新登录
            DLOG("找不到cookie信息，请重新登录");
            return http_response(conn, false, websocketpp::http::status_code::bad_request, "找不到cookie信息，请重新登录");
        }

        // 2. 从cookie中获取sessionID -- 封装一个接口并且调用
        std::string sessionID;
        bool ret = get_sessionID_from_cookie(cookie_str, "SSID", sessionID);
        if(ret == false)
        {
            // cookie中没有会话id，则返回错误，让客户端重新登录
            DLOG("找不到会话id，请重新登录");
            return http_response(conn, false, websocketpp::http::status_code::bad_request, "找不到会话id，请重新登录");
        }

        // 3. 在session管理对象中查找对应的session信息
        session_ptr sp = _session.get_session_by_sesssionID(std::stol(sessionID));
        if(sp.get() == nullptr)
        {
            // 在本地会话管理中找不到该会话，则认为登录已经过期，需要重新登录
            DLOG("会话过期，请重新登录");
            return http_response(conn, false, websocketpp::http::status_code::bad_request, "会话过期，请重新登录");
        }

        // 4. 先获取用户id，然后从数据库中取出用户信息
        uint64_t userid = sp->getUserID();
        Json::Value userinfo;
        ret = _user_table.select_by_id(userid, userinfo);
        if(ret == false)
        {
            // 获取用户信息失败，返回错误：找不到用户信息
            DLOG("找不到用户信息，请重新登录");
            return http_response(conn, false, websocketpp::http::status_code::bad_request, "找不到用户信息，请重新登录");
        }

        // 5. 将用户信息进行序列化后发送给客户端 -- 一般不会序列化失败，所以这里不判断
        std::string body;
        json_util::serialize(userinfo, body);
        conn->set_body(body);
        conn->set_status(websocketpp::http::status_code::ok);
        conn->append_header("Content-Type", "application/json");

        // 6. 刷新session过期时间，也就是重新设置过期时间
        _session.set_session_expire_time(sp->getSessionID(), SESSION_EXPIRE_TIME);
    }

private:
/*                 websocket连接建立成功处理函数                 */

    // websocket长连接建立成功之后的处理函数 -- 根据请求类型不同分为游戏大厅和游戏房间连接
    void open_callback(websocketpp::connection_hdl hdl)
    {
        // 1. 获取http请求的uri，用于区分不同的请求类型
        wsserver_t::connection_ptr conn = _server.get_con_from_hdl(hdl);
        websocketpp::http::parser::request req = conn->get_request();
        std::string uri = req.get_uri();

        // 2. 根据不同的请求类型调用对应的处理函数
        if(uri == "/hall")      
            return open_game_hall(conn); // 游戏大厅的长连接建立成功处理
        else if(uri == "/room")  
            return open_game_room(conn); // 游戏房间的长连接建立成功处理
    }

    // 游戏大厅的长连接建立成功处理函数
    void open_game_hall(wsserver_t::connection_ptr& conn)
    {
        // 1. 登录验证--判断当前客户端是否已经成功登录
        session_ptr sp = websocket_get_session(conn);
        if (sp.get() == nullptr)
            return;

        // 2. 判断当前客户端是否是重复登录
        if (_online_user.isInHall(sp->getUserID()) || _online_user.isInRoom(sp->getUserID())) 
            return websocket_response(conn, false, "hall_ready", "玩家重复登录!");

        // 3. 将当前客户端以及连接加入到游戏大厅
        _online_user.enterHall(sp->getUserID(), conn);

        //4. 给客户端响应游戏大厅连接建立成功
        websocket_response(conn, true, "hall_ready", "游戏大厅连接建立成功");

        // 5. 记得将session设置为永久存在
        _session.set_session_expire_time(sp->getSessionID(), SESSION_FOREVER);
    }

    // 游戏大厅的长连接建立成功处理函数
    void open_game_room(wsserver_t::connection_ptr& conn)
    {
        // 1. 获取当前客户端的session
        session_ptr sp = websocket_get_session(conn);
        if(sp.get() == nullptr)
            return;

        // 2. 判断当前用户是否重复登陆---在线用户管理
        if(_online_user.isInHall(sp->getUserID()) || _online_user.isInRoom(sp->getUserID()))
        {
            return websocket_response(conn, false, "room_ready", "玩家重复登录！");
        }

        // 3. 判断当前用户是否已经创建好了房间 --- 房间管理
        room_ptr rp = _room.getRoom_ByUserID(sp->getUserID());
        if(rp.get() == nullptr)
        {
            return websocket_response(conn, false, "room_ready", "没有找到玩家的房间信息");
        }

        // 4. 将当前用户添加到在线用户管理的游戏房间中
        _online_user.enterRoom(sp->getUserID(), conn);

        // 5. 将session重新设置为永久存在
        _session.set_session_expire_time(sp->getSessionID(), SESSION_FOREVER);

        // 6. 回复房间内玩家已经准备完毕
        Json::Value resp_json;
        resp_json["optype"] = "room_ready";
        resp_json["result"] = true;
        resp_json["room_id"] = (Json::UInt64)rp->getRoomID();
        resp_json["uid"] = (Json::UInt64)sp->getUserID();
        resp_json["white_id"] = (Json::UInt64)rp->getWhiteID();
        resp_json["black_id"] = (Json::UInt64)rp->getBlackID();
        std::string body;
        json_util::serialize(resp_json, body);
        conn->send(body);
    }

private:
/*                 websocket断开连接处理函数                 */

    // websocket长连接建立断开之前的处理函数 -- 分为游戏大厅和游戏房间连接
    void close_callback(websocketpp::connection_hdl hdl)
    {
        // 1. 获取http请求的uri，用于区分不同的请求类型
        wsserver_t::connection_ptr conn = _server.get_con_from_hdl(hdl);
        websocketpp::http::parser::request req = conn->get_request();
        std::string uri = req.get_uri();

        // 2. 根据不同的请求类型调用对应的处理函数
        if(uri == "/hall")      // 游戏大厅的长连接断开前处理
            return close_game_hall(conn);
        else if(uri == "/room")  // 游戏房间的长连接断开前处理
            return close_game_room(conn);
    }

    // 游戏大厅长连接断开前的处理
    void close_game_hall(wsserver_t::connection_ptr& conn)
    {
        // 1. 获取session信息，在子函数中顺便进行登录验证
        session_ptr sp = websocket_get_session(conn);
        if (sp.get() == nullptr)
            return;

        // 2. 将玩家从游戏大厅中移除
        _online_user.exitHall(sp->getUserID());

        // 3. 将session恢复生命周期的管理，设置定时销毁
        _session.set_session_expire_time(sp->getSessionID(), SESSION_EXPIRE_TIME);
    }

    // 游戏房间长连接断开前的处理
    void close_game_room(wsserver_t::connection_ptr& conn)
    {
        // 1. 获取session信息，在子函数中顺便进行登录验证
        session_ptr sp = websocket_get_session(conn);
        if (sp.get() == nullptr)
            return;

        // 2. 将玩家从在线用户管理中移除
        _online_user.exitRoom(sp->getUserID());

        // 3. 将session回复生命周期的管理，设置定时销毁
        _session.set_session_expire_time(sp->getSessionID(), SESSION_EXPIRE_TIME);

        // 4. 将玩家从游戏房间中移除，房间中所有用户退出了就会销毁房间
        _room.removeUser(sp->getUserID());
    }

private:
/*                 websocket收发消息处理函数                 */

    // websocket长连接通信处理
    void message_callback(websocketpp::connection_hdl hdl, wsserver_t::message_ptr msgptr)
    {
        // 1. 获取http请求的uri，用于区分不同的请求类型
        wsserver_t::connection_ptr conn = _server.get_con_from_hdl(hdl);
        websocketpp::http::parser::request req = conn->get_request();
        std::string uri = req.get_uri();

        // 2. 根据不同的请求类型调用对应的处理函数
        if(uri == "/hall")      
            return message_in_hall(conn, msgptr); // 游戏大厅的收发信息处理
        else if(uri == "/room")  
            return message_in_room(conn, msgptr); // 游戏房间的收发信息处理
    }

    // 游戏大厅的通信处理
    void message_in_hall(wsserver_t::connection_ptr& conn, wsserver_t::message_ptr& msgptr)
    {
        // 1. 获取session信息，在子函数中完成身份验证
        session_ptr sp = websocket_get_session(conn);
        if(sp.get() == nullptr)
            return;

        // 2. 获取消息的请求数据 
        std::string req_body = msgptr->get_payload();
        Json::Value req_json;
        bool ret = json_util::unserialize(req_body, req_json);
        if(ret == false)
            return websocket_response(conn, false, "", "请求信息解析失败");

        // 3. 根据请求进行处理
        if (!req_json["optype"].isNull() && req_json["optype"].asString() == "match_start")
        {
            //  开始对战匹配：通过匹配模块，将用户添加到匹配队列中
            _matcher.addUser(sp->getUserID());
            return websocket_response(conn, true, "match_start", "开始匹配成功");
        }
        else if (!req_json["optype"].isNull() && req_json["optype"].asString() == "match_stop") 
        {
            //  停止对战匹配：通过匹配模块，将用户从匹配队列中移除
            _matcher.delUser(sp->getUserID());
            return websocket_response(conn, true, "match_stop", "停止匹配成功");
        }

        return websocket_response(conn, false, "unknow", "请求类型未知");
    }

    // 游戏房间的通信处理
    void message_in_room(wsserver_t::connection_ptr& conn, wsserver_t::message_ptr& msgptr)
    {
        // 1. 获取session信息，在子函数中顺便进行登录验证
        session_ptr sp = websocket_get_session(conn);
        if (sp.get() == nullptr)
            return;

        // 2. 获取客户端房间信息
        room_ptr rp = _room.getRoom_ByUserID(sp->getUserID());
        if(rp.get() == nullptr)
        {
            DLOG("房间--没有找到玩家房间信息");
            return websocket_response(conn, false, "unknow", "没有找到玩家的房间信息");
        }

        // 3. 对消息进行反序列化
        Json::Value req_json;
        std::string req_body = msgptr->get_payload();
        bool ret = json_util::unserialize(req_body, req_json);
        if(ret == false)
        {
            DLOG("房间-反序列化请求失败");
            return websocket_response(conn, false, "unknow", "请求解析失败");
        }
        DLOG("房间：收到房间请求，开始处理....");

        // 4. 通过房间模块进行消息请求的处理
        rp->request_handle(req_json);
    }

private:
/*                 辅助处理函数                 */
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

    // websocket响应处理通用函数
    void websocket_response(wsserver_t::connection_ptr& conn, 
                            bool result, 
                            const std::string& optype,
                            const std::string& reason) 
    {
        Json::Value resp_json;
        resp_json["optype"] = optype;
        if(!optype.empty()) resp_json["result"] = result;
        if(!reason.empty()) resp_json["reason"] = reason;
        std::string body;
        json_util::serialize(resp_json, body);
        conn->send(body);
    }

    // 获取session_ptr -- 顺便验证登录
    session_ptr websocket_get_session(wsserver_t::connection_ptr& conn)
    {
        // 1. 获取请求信息中的Cookie，从Cookie中获取ssid
        std::string cookie_str = conn->get_request_header("Cookie");
        if (cookie_str.empty()) 
        {
            // 如果没有cookie，返回错误：没有cookie信息，让客户端重新登录
            websocket_response(conn, false, "hall_ready", "没有找到cookie信息，需要重新登录");
            return session_ptr();
        }
        // 2. 从cookie中取出ssid
        std::string ssid_str;
        bool ret = get_sessionID_from_cookie(cookie_str, "SSID", ssid_str);
        if (ret == false) 
        {
            // cookie中没有ssid，返回错误：没有ssid信息，让客户端重新登录
            websocket_response(conn, false, "hall_ready", "没有找到SSID信息，需要重新登录");
            return session_ptr();
        }

        // 3. 在session管理中查找对应的会话信息
        session_ptr sp = _session.get_session_by_sesssionID(std::stol(ssid_str));
        if (sp.get() == nullptr) 
        {
            // 没有找到session，则认为登录已经过期，需要重新登录
            websocket_response(conn, false, "hall_ready", "登录已经过期，需要重新登录");
            return session_ptr();
        }
        return sp;
    }

    // 从cookie中获取sessionID函数
    bool get_sessionID_from_cookie(const std::string& cookie_str, const std::string& key, std::string &val)
    {
        // Cookie: SSID=XXX; path=/; 
        //1. 以 ; 作为分隔符，对字符串进行分割，得到各个单个的cookie信息
        std::string sep = "; ";
        std::vector<std::string> cookie_arr;
        string_util::split(cookie_str, sep, cookie_arr);
        for (auto &str : cookie_arr) {
            //2. 对单个cookie字符串，以 = 为分隔符进行分割，得到key和val
            std::vector<std::string> tmp_arr;
            string_util::split(str, "=", tmp_arr);
            if (tmp_arr.size() != 2) { continue; }
            if (tmp_arr[0] == key) {
                val = tmp_arr[1];
                return true;
            }
        }
        return false;
    }
};

#endif