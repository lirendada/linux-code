#ifndef __MY_ROOM_H__
#define __MY_ROOM_H__
#include "util.hpp"
#include "online.hpp"
#include "logger.hpp"
#include "db.hpp"
#include <vector>

#define BOARD_ROW 15
#define BOARD_COL 15
#define WHITE_CHESS 1
#define BLACK_CHESS 2
typedef enum { GAME_START, GAME_OVER }room_status;

class room
{
private:
    uint64_t _room_id;                   // 房间id
    room_status _status;                 // 房间状态
    int _player_num;                     // 房间玩家个数
    uint64_t _white_id;                  // 白棋id
    uint64_t _black_id;                  // 黑棋id
    user_table* _table_user;             // 数据库用户信息管理句柄
    online_manager* _online_user;        // 在线用户管理句柄
    std::vector<std::vector<int>> _board; // 棋盘
public:
    room(uint64_t room_id, user_table* table_user, online_manager* online_user)
        : _room_id(room_id), _table_user(table_user), _online_user(online_user),
          _player_num(0), _board(BOARD_ROW, std::vector<int>(BOARD_COL, 0))
    {
        DLOG("%lu 房间创建成功！", _room_id);
    }
    ~room()
    {
        DLOG("%lu 房间销毁成功！", _room_id);
    }

    //  获取成员变量的函数
    uint64_t getRoomID() { return _room_id; }
    room_status getStatus() { return _status; }
    int getPlayerNum() { return _player_num; }
    uint64_t getWhiteID() { return _white_id; }
    uint64_t getBlackID() { return _black_id; }

    // 添加玩家接口
    void add_white(uint64_t uid) { _white_id = uid; _player_num++; }
    void add_black(uint64_t uid) { _black_id = uid; _player_num++; }

    // 总的请求处理函数，在函数内部区分请求类型，根据不同的请求调用不同的处理函数，得到响应进行广播
    void request_handle(Json::Value& req)
    {
        Json::Value response = req;

        // 1. 判断当前请求的房间号是否与当前房间的房间号匹配
        uint64_t room_id = req["room_id"].asUInt64();
        if(_room_id != room_id)
        {
            response["result"] = false;
            response["reason"] = "游戏房间不匹配";
            broadcast(response); // 广播信息
            return;
        }

        // 2. 根据不同的请求调用不同的处理函数
        if(req["optype"].asCString() == "put_chess")
        {
            response = chess_handle(req);
            // 判断如果不是平局，那么就得更新数据库
            if(response["winner"].asUInt64() != 0)
            {
                uint64_t winner_id = response["winner"].asUInt64();
                uint64_t loser_id = (winner_id == _white_id ? _black_id : _white_id);
                _table_user->win(winner_id);
                _table_user->lose(loser_id);

                // 记得还得修改房间状态
                _status = GAME_OVER;
            }
        }
        else if(req["optype"].asCString() == "chat")
        {
            response = chat_handle(req);
        }
        else
        {
            response["result"] = false;
            response["reason"] = "未知请求类型";
        }

        // 3. 广播信息。广播之前先日志输出一下
        std::string body;
        json_util::serialize(response, body);
        DLOG("房间-广播动作: %s", body.c_str());
        broadcast(response);
    }

    // 处理下棋动作，返回下棋响应
    Json::Value chess_handle(Json::Value& req)
    {
        Json::Value response = req;

        // 1. 判断房间中两个玩家是否都在线，任意一个不在线的话就是对方获胜
        uint64_t uid = req["uid"].asUInt64();
        int chess_row = req["row"].asInt();
        int chess_col = req["col"].asInt();
        if(!_online_user->isInRoom(_white_id))
        {
            response["result"] = true;
            response["reason"] = "运气真好！对方掉线，不战而胜！";
            response["winner"] = (Json::UInt64)_black_id;
            return response;
        }
        if(!_online_user->isInRoom(_black_id))
        {
            response["result"] = true;
            response["reason"] = "运气真好！对方掉线，不战而胜！";
            response["winner"] = (Json::UInt64)_white_id;
            return response;
        }

        // 2. 根据走棋位置，判断当前走棋是否合理（比如位置是否已经被占用了）
        if(_board[chess_row][chess_col] != 0)
        {
            response["result"] = false;
            response["reason"] = "当前位置已经有了其他棋子！";
            return response;
        }
        int chess_color = (uid == _white_id ? WHITE_CHESS : BLACK_CHESS);
        _board[chess_row][chess_col] = chess_color;

        // 3. 判断是否有玩家胜利（从当前走棋位置开始判断是否存在五星连珠）
        uint64_t winner_id = check_win(chess_row, chess_col, chess_color);
        if(winner_id != 0)
            response["reason"] = "五星连珠，战无敌！";
        response["result"] = true;
        response["winner"] = (Json::UInt64)winner_id;
        return response;
    }

    // 处理聊天动作，返回下棋响应
    Json::Value chat_handle(Json::Value& req)
    {
        Json::Value response = req;

        // 1. 检测消息中是否包括敏感词
        std::string msg = req["message"].asString();
        size_t pos = msg.find("垃圾");
        if(pos != std::string::npos)
        {
            response["result"] = false;
            response["reason"] = "消息中包含敏感词，不能发送！";
            return response;
        }

        // 2. 没有异常问题的话，将"result"字段设为true然后返回即可
        response["result"] = true;
        return response;
    }

    // 处理玩家退出房间的动作 -- 不属于request_handle来管的，因为退出动作在这里并不属于请求类型
    void exit_handle(uint64_t uid)
    {
        Json::Value response;

        // 1. 如果是在下棋时候退出，则对方获胜；如果下棋结束了退出，则是正常退出，不用额外处理
        if(_status == GAME_START)
        {
            response["optype"] = "put_chess";
            response["result"] = true;
            response["reason"] = "对方掉线，不战而胜！";
            response["room_id"] = (Json::UInt64)_room_id;
            response["uid"] = (Json::UInt64)uid;
            response["row"] = -1;
            response["col"] = -1;

            uint64_t winner_id = (Json::UInt64)(uid == _white_id ? _black_id : _white_id);
            response["winner"] = (Json::UInt64)winner_id;

            // 更新数据库
            uint64_t loser_id = winner_id == _white_id ? _black_id : _white_id;
            _table_user->win(winner_id);
            _table_user->lose(loser_id);

            // 设置房间状态
            _status = GAME_OVER;

            // 广播信息给房间内玩家
            broadcast(response);
        }

        // 2. 只要退出了，都得对玩家人数进行减少
        _player_num--;
    }

    // 将特定的信息广播给房间中所有玩家
    void broadcast(Json::Value& req)
    {
        // 1. 对要响应的信息进行序列化，将Json::Value中的数据序列化成为json格式字符串
        std::string body;
        bool ret = json_util::serialize(req, body);

        // 2. 获取房间中的用户的通信连接，并且响应信息
        wsserver_t::connection_ptr white_conn = _online_user->get_conn_from_room(_white_id);
        if(white_conn.get() != nullptr)
            white_conn->send(body);
        else
            DLOG("房间-白棋玩家连接获取失败");
        
        wsserver_t::connection_ptr black_conn = _online_user->get_conn_from_room(_black_id);
        if(black_conn.get() != nullptr)
            black_conn->send(body);
        else
            DLOG("房间-黑棋玩家连接获取失败");
    }

private:
    // 检查是否下棋完会有玩家胜利
    // 返回值：0表示没有玩家胜利，1表示白棋胜利，2表示黑棋胜利
    uint64_t check_win(int row, int col, int color)
    {   
        // 从当前行、列、左斜、右斜线上判断是否有连续五个相同颜色的棋子
        // 规定向上和右为1，向下和左为-1，不变为0
        if(five(row, col, color, 0, 1) ||
            five(row, col, color, 1, 0) ||
            five(row, col, color, -1, -1) ||
            five(row, col, color, -1, 1))
        {
            return (color == WHITE_CHESS ? _white_id : _black_id);
        }
        return 0;
    }

    bool five(int row, int col, int color, int row_offset, int col_offset)
    {
        int count = 0;
        int tmprow = row, tmpcol = col;
        // 先正向检查
        while(tmprow >= 0 && tmprow < BOARD_ROW &&
                tmpcol >= 0 && tmpcol < BOARD_COL &&
                _board[tmprow][tmpcol] == color)
        {
            // 同色棋子数量++
            count++;
            // 检索位置继续向后偏移
            tmprow += row_offset;
            tmpcol += col_offset;
        }

        // 再反向检查
        tmprow = row - row_offset, tmpcol = col - col_offset;
        while(tmprow >= 0 && tmprow < BOARD_ROW &&
                tmpcol >= 0 && tmpcol < BOARD_COL &&
                _board[tmprow][tmpcol] == color)
        {
            // 同色棋子数量++
            count++;
            // 检索位置继续向后偏移
            row -= row_offset;
            col -= col_offset;
        }

        return (count >= 5);
    }
};


#endif