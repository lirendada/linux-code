#include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <unistd.h>

using func_t = std::function<void()>;        // 超时任务的函数类型，由使用者传入
using remove_t = std::function<void()>;      // 用于释放weak_ptr的函数类型，由TimerWheel传入
 
// 定时任务类，封装一个定时任务
class TimerTask
{
private:
    uint64_t _id;       // 当前超时任务类的ID
    uint32_t _timeout;  // 超时时间
    func_t _task;       // 超时任务
    remove_t _remove;   // 释放TimerWheel中的weak_ptr
    bool _cancel;       // 为true表示要取消任务，为false表示正常执行任务
public:
    TimerTask(uint64_t id, uint32_t timeout, const func_t& task) : _id(id), _timeout(timeout), _task(task), _cancel(false) {}

    ~TimerTask()
    {
        // 如果没有取消任务，才执行释放函数
        if(_cancel == false)
        {
            // 析构函数进行超时任务以及weak_ptr释放函数的执行
            _task();
            _remove();
        }
    }

    uint64_t get_id() { return _id; }
    uint32_t get_timeout() { return _timeout; }
    void set_remove(const remove_t& remove) { _remove = remove; }
    void set_cancel() { _cancel = true; }
};

// 时间轮类
class TimerWheel
{
    using shared_t = std::shared_ptr<TimerTask>;
    using weak_t = std::weak_ptr<TimerTask>;
private:
    int _tick;                                     // 当前的时间轮秒数，每一秒就往后走一步
    int _capacity;                                 // 时间轮数组大小，即时间轮的周期
    std::vector<std::vector<shared_t>> _wheel;     // 时间轮数组
    std::unordered_map<uint64_t, weak_t> _table;   // 保存所有定时任务对象的weak_ptr，这样才能在不影响shared_ptr计数器的同时，获取其shared_ptr
public:
    TimerWheel() : _tick(0), _capacity(60), _wheel(_capacity) {}

    // 时间运行函数
    void run_timer()
    {
        // 一秒钟走一步，每次将到达的位置处的shared_ptr进行清空，如果是最后一次任务的话会自动调用其析构函数进行释放
        _tick = (_tick + 1) % _capacity;
        _wheel[_tick].clear();
    }

    // 添加定时任务
    void add_timertask(uint64_t id, uint32_t timeout, const func_t& task)
    {
        // 1. 创建一个定时任务，由智能指针管理
        shared_t newtask(new TimerTask(id, timeout, task));
        if(newtask.get() == nullptr)
            return;
        
        // 2. 设置释放函数
        newtask->set_remove(std::bind(&TimerWheel::remove_timer, this, id));

        // 3. 向时间轮数组中添加定时任务
        int pos = (_tick + newtask->get_timeout()) % _capacity; // 注意需要取模，防止越界
        _wheel[pos].push_back(newtask);

        // 4. 将定时任务交给哈希表管理，记得要使用weak_ptr才不会导致计数增加
        _table[id] = weak_t(newtask);
    }

    // 刷新定时任务
    void refresh_timertask(uint64_t id)
    {
        // 1. 首先通过哈希表找到保存的超时任务的weak_ptr
        auto it = _table.find(id);
        if(it == _table.end())
            return;
        
        // 2. 通过weak_ptr构造一个shared_ptr出来
        shared_t refresh_task(it->second.lock());

        // 3. 将刷新任务添加到时间轮数组中
        int pos = (_tick + refresh_task->get_timeout()) % _capacity; // 注意需要取模，防止越界
        _wheel[pos].push_back(refresh_task);

        // 4. 将定时任务交给哈希表管理，记得要使用weak_ptr才不会导致计数增加
        _table[id] = weak_t(refresh_task);
    }

    // 取消定时任务
    void cancel_timertask(uint64_t id)
    {
        // 先判断在不在哈希表中
        auto it = _table.find(id);
        if(it == _table.end())
            return;

        // 先拿到shared_ptr，再通过其取消任务
        shared_t st(it->second.lock()); 
        if(st.get() != nullptr)
            st->set_cancel();   
    }
private:
    // 在哈希表中去除并且释放weak_ptr
    void remove_timer(uint64_t id)
    {
        // 先判断在不在哈希表中
        auto it = _table.find(id);
        if(it == _table.end())
            return;

        _table.erase(id);
    }
};

class Test
{
public:
    Test() { std::cout << "Test构造完成" << std::endl; }
    ~Test() { std::cout << "Test析构完成" << std::endl; }
};

void Delete(Test* t)
{
    delete t;
}

int main()
{
    std::unique_ptr<TimerWheel> tw(new TimerWheel);
    Test* t = new Test;
    tw->add_timertask(1, 5, std::bind(Delete, t));

    for(int i = 1; i <= 3; ++i)
    {
        sleep(1);
        tw->refresh_timertask(1);
        std::cout << "该任务被刷新了" << i << "次，需要在最新的5秒后才执行超时任务" << std::endl;
    }
    
    tw->cancel_timertask(1);
    while(true)
    {
        sleep(1);
        std::cout << "定时器执行中" << std::endl;
        tw->run_timer();
    }
    return 0;
}