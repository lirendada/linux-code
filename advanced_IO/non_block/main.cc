#include <cstdio>
#include <vector>
#include <functional>
#include "util.hpp"

using func_t = std::function<void()>; 

#define INIT(v) do{\
    v.push_back(DownLoad);\
    v.push_back(Print);\
    v.push_back(ExecuteSql);\
}while(0)

#define RUN(callbacks) do{\
    for(const auto & e : callbacks) e();\
}while(0)

int main()
{
    std::vector<func_t> v; 
    INIT(v); 

    // 设置非阻塞等待
    setNonBlock(0);

    char buffer[1024]; 
    while(true)
    {
        printf(">>> ");
        fflush(stdout); 

        ssize_t n = read(0, buffer, sizeof(buffer) - 1);
        if(n > 0)
        {
            buffer[n - 1] = 0;
            std::cout << "echo: " << buffer << std::endl;
        }
        else if(n == 0)
        {
            std::cout << "read end" << std::endl;
            break;
        }
        else
        {}

        RUN(v);
        std::cout << "----------------------" << std::endl;
        sleep(2); // 睡眠一会
    }
    return 0;
}