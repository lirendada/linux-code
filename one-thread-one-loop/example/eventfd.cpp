#include <iostream>
#include <sys/eventfd.h>
#include <unistd.h>
using namespace std;

int main()
{
    // 创建一个eventfd
    int efd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if(efd < 0)
    {
        perror("eventfd error");
        return -1;
    }

    // 对eventfd进行写
    uint64_t val = 1;
    write(efd, &val, sizeof(uint64_t));
    write(efd, &val, sizeof(uint64_t));

    // 此时再对eventfd进行读
    uint64_t ret = 0;
    read(efd, &ret, sizeof(uint64_t));
    cout << ret << endl;

    close(efd);
    return 0;
}