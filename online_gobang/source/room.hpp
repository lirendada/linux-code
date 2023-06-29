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
        DLOG("总的请求处理函数开始");
        Json::Value response;

        // 1. 首先需要判断当前请求的房间号是否与当前房间的房间号匹配
        uint64_t room_id = req["room_id"].asUInt64();
        if(_room_id != room_id)
        {
            DLOG("房间号不匹配");
            response["optype"] = req["optype"].asString();
            response["result"] = false;
            response["reason"] = "游戏房间不匹配";
            broadcast(response); // 广播信息
            return;
        }

        // 2. 根据不同的请求调用不同的处理函数
        //	  是根据json数据字段中的"optype"来确定是何种请求的
        if(req["optype"].asString() == "put_chess")
        {
            DLOG("收到是下棋请求");
            response = chess_handle(req);
            // 判断如果不是平局，那么就得更新数据库，因为有人获胜了
            if(response["winner"].asUInt64() != 0)
            {
                DLOG("有人胜利");
                uint64_t winner_id = response["winner"].asUInt64();
                uint64_t loser_id = (winner_id == _white_id ? _black_id : _white_id);
                _table_user->win(winner_id);
                _table_user->lose(loser_id);

                // 记得还得修改房间状态
                _status = GAME_OVER;
            }
        }
        else if(req["optype"].asString() == "chat")
        {
            DLOG("收到是聊天请求");
            response = chat_handle(req);
        }
        else
        {
            DLOG("未知请求");
            response["optype"] = req["optype"].asString();
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
        if(_online_user->isInRoom(_white_id) == false)
        {
            DLOG("对方掉线");
            response["result"] = true;
            response["reason"] = "运气真好！对方掉线，不战而胜！";
            response["winner"] = (Json::UInt64)_black_id;
            return response;
        }
        if(_online_user->isInRoom(_black_id) == false)
        {
            DLOG("对方掉线");
            response["result"] = true;
            response["reason"] = "运气真好！对方掉线，不战而胜！";
            response["winner"] = (Json::UInt64)_white_id;
            return response;
        }

        // 2. 根据走棋位置，判断当前走棋是否合理（比如位置是否已经被占用了）
        if(_board[chess_row][chess_col] != 0)
        {
            DLOG("该位置已有棋子");
            response["result"] = false;
            response["reason"] = "当前位置已经有了其他棋子！";
            return response;
        }
        int chess_color = (uid == _white_id ? WHITE_CHESS : BLACK_CHESS);
        _board[chess_row][chess_col] = chess_color;

        // 3. 判断是否有玩家胜利（从当前走棋位置开始判断是否存在五星连珠）
        DLOG("判断是否有玩家胜利：开始");
        uint64_t winner_id = check_win(chess_row, chess_col, chess_color);
        if(winner_id != 0)
            response["reason"] = "五星连珠，战无敌！";
        response["result"] = true;
        response["winner"] = (Json::UInt64)winner_id;
        DLOG("下棋操作结束");
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
        json_util::serialize(req, body);

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
        // 从下棋位置的四个不同方向上检测是否出现了5个及以上相同颜色的棋子（横行，纵列，正斜，反斜）
        if (five(row, col, 0, 1, color) || 
            five(row, col, 1, 0, color) ||
            five(row, col, -1, 1, color)||
            five(row, col, -1, -1, color)) {
            //任意一个方向上出现了true也就是五星连珠，则设置返回值
            return color == WHITE_CHESS ? _white_id : _black_id;
        }
        return 0;
    }

    bool five(int row, int col, int row_offset, int col_offset, int color)
    {
        int count = 1;
        int tmprow = row + row_offset;
        int tmpcol = col + col_offset;
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
        tmprow = row - row_offset;
        tmpcol = col - col_offset;
        while(tmprow >= 0 && tmprow < BOARD_ROW &&
                tmpcol >= 0 && tmpcol < BOARD_COL &&
                _board[tmprow][tmpcol] == color)
        {
            // 同色棋子数量++
            count++;
            // 检索位置继续向后偏移
            tmprow -= row_offset;
            tmpcol -= col_offset;
        }

        return (count >= 5);
    }
};


using room_ptr = std::shared_ptr<room>; // 声明一个房间类的智能指针类型

class room_manager
{
private:
    user_table* _user_tb;         // 数据库用户信息表管理句柄                          
    online_manager* _online_user; // 在线用户管理句柄
    uint64_t count;               // 房间 ID 分配计数器
    std::mutex _mtx;              // 互斥锁
    std::unordered_map<uint64_t, room_ptr> rid_rinfo_hash; // 房间id与房间信息的管理哈希表
    std::unordered_map<uint64_t, uint64_t> uid_rid_hash;   // 用户id与房间id的管理哈希表
public:
    // 构造函数和析构函数
    room_manager(user_table* user_tb, online_manager* online_user)
        : _user_tb(user_tb), _online_user(online_user)
    { DLOG("房间管理模块初始化完毕！"); }
    ~room_manager() { DLOG("房间管理模块即将销毁！"); }

    // 增加房间函数，并返回该房间的智能指针管理对象
    room_ptr addRoom(uint64_t uid1, uint64_t uid2)
    {
        // 背景：两个用户在游戏大厅中进行对战匹配，匹配成功后创建房间
        // 1. 校验两个用户是否都还在游戏大厅中，只有都在才需要创建房间
        if(_online_user->isInHall(uid1) == false || _online_user->isInHall(uid2) == false)
        {
            DLOG("有用户不在大厅中，创建房间失败!");
            return room_ptr();
        }

        // 2. 创建房间，将用户信息添加到房间中
        std::unique_lock<std::mutex> lock(_mtx); // 从这里开始的操作都要加锁保护
        room_ptr rp(new room(count, _user_tb, _online_user));
        rp->add_white(uid1);
        rp->add_black(uid2);

        // 3. 将房间信息管理起来，记得最后要对计数器++
        uid_rid_hash[uid1] = count;
        uid_rid_hash[uid2] = count;
        rid_rinfo_hash[count] = rp;
        count++; // 这步别忘了

        // 4. 返回房间信息
        return rp;
    }

    // 通过房间ID获取房间信息
    room_ptr getRoom_ByRoomID(uint64_t roomID)
    {
        std::unique_lock<std::mutex> lock(_mtx); // 需要加锁保护
        auto ret = rid_rinfo_hash.find(roomID);
        if(ret == rid_rinfo_hash.end())
        {
            return room_ptr();
        }
        return ret->second;
    }

    // 通过用户ID获取房间信息
    room_ptr getRoom_ByUserID(uint64_t userID)
    {
        std::unique_lock<std::mutex> lock(_mtx); // 需要加锁保护

        // 1. 先通过用户ID查找房间ID
        auto ret = uid_rid_hash.find(userID);
        if(ret == uid_rid_hash.end())
        {
            return room_ptr();
        }

        // 2. 再通过房间ID获取房间信息
        // 注意：不能直接调用getRoom_ByRoomID来获取房间信息，因为会重复加锁导致死锁
        auto it = rid_rinfo_hash.find(ret->second);
        if(it == rid_rinfo_hash.end())
        {
            return room_ptr();
        }
        return it->second;
    }

    // 通过房间ID销毁房间
    void removeRoom(uint64_t roomID)
    {
        // 因为房间信息是通过shared_ptr在哈希表中进行管理，因此只要将shared_ptr从哈希表中移除
        // 则当shared_ptr计数器==0，外界没有对房间信息进行操作保存的情况下就会释放
        // 但是因为房间中的用户信息等也要移除，不然会造成内存泄漏问题
        // 所以我们要先移除必须的用户信息再移除房间管理信息

        // 1. 通过房间ID，获取房间信息
        room_ptr rp = getRoom_ByRoomID(roomID);
        if(rp.get() == nullptr)
            return;

        // 2. 通过房间信息，获取房间中所有用户的ID
        uint64_t uid1 = rp->getBlackID();
        uint64_t uid2 = rp->getWhiteID();

        // 3. 移除房间管理中的用户信息
        std::unique_lock<std::mutex> lock(_mtx); // 加锁保护
        uid_rid_hash.erase(uid1);
        uid_rid_hash.erase(uid2);

        // 4. 移除房间管理信息
        rid_rinfo_hash.erase(roomID);
    }

    // 删除房间中指定用户，如果房间中没有用户了，则销毁房间，用户连接断开时被调用
    void removeUser(uint64_t userID)
    {
        // 1. 首先获取房间信息
        room_ptr rp = getRoom_ByUserID(userID);
        if(rp.get() == nullptr) 
            return;

        // 2. 处理玩家退出动作
        rp->exit_handle(userID);

        // 3. 判断一下房间是否没有用户了，没有的话销毁房间
        if(rp->getPlayerNum() == 0)
            removeRoom(rp->getRoomID());
    }
};

#endif