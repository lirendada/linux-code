#include "Block_queue.hpp"
#include "task.hpp"
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string>

// 封装一下两个阻塞队列
template <class C, class S> // C:计算  S:存储
class BlockQueues
{
public:
    BlockQueues()
    {
        cal_bq = new BlockQueue<CalTask>();
        save_bq = new BlockQueue<SaveTask>();
    }
    ~BlockQueues()
    {
        delete cal_bq;
        delete save_bq;
    }
public:
    BlockQueue<C> *cal_bq;
    BlockQueue<S> *save_bq;
};

void* Productor(void* bqs)
{
    BlockQueue<CalTask>* bq = (static_cast<BlockQueues<CalTask, SaveTask>*>(bqs))->cal_bq;
    while(true)
    {
        // 生产任务
        int x = rand() % 100 + 1;
        int y = rand() % 100;
        char op = oper[rand() % oper.size()];

        CalTask ct(x, y, op, caltask);
        bq->put(ct);
        std::cout << "productor thread, 生产计算任务: " << ct.toTaskString() << std::endl;

        sleep(1);
    }
    return nullptr;
}

void* Consumer(void* bqs)
{
    BlockQueue<CalTask>* bq = (static_cast<BlockQueues<CalTask, SaveTask>*>(bqs))->cal_bq;
    BlockQueue<SaveTask>* save_bq = (static_cast<BlockQueues<CalTask, SaveTask>*>(bqs))->save_bq;
    while(true)
    {
        // 消费任务
        CalTask t;
        bq->take(&t);

        // 执行任务
        std::string result = t(); // 任务非常耗时！！
        std::cout << "Consumer thread，完成计算任务: " << result << " ... done"<< std::endl;

        // 生产任务
        SaveTask st(result, savetask);
        save_bq->put(st);
        std::cout << "Consumer thread，推送存储任务完成..." << std::endl; 
        sleep(1);
    }
    return nullptr;
}

void* Saver(void* bqs)
{
    BlockQueue<SaveTask>* save_bq = (static_cast<BlockQueues<CalTask, SaveTask>*>(bqs))->save_bq;
    while(true)
    {
        // 消费任务
        SaveTask st;
        save_bq->take(&st);

        // 执行任务
        std::cout << "save thread，保存任务完成..." << std::endl; 
        st();
    }
    return nullptr;
}

int main()
{
    srand((unsigned int)time(nullptr) ^ getpid()); // 生成随机种子

    BlockQueues<CalTask, SaveTask> bqs;
    pthread_t consumer, productor, saver;
    pthread_create(&productor, nullptr, Productor, &bqs);
    pthread_create(&consumer, nullptr, Consumer, &bqs);
    pthread_create(&saver, nullptr, Saver, &bqs);

    pthread_join(consumer, nullptr);
    pthread_join(productor, nullptr);
    pthread_join(saver, nullptr);
    return 0;
}