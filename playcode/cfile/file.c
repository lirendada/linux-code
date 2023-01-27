// #include <stdio.h>

// int main()
// {
//     FILE* fp = fopen("log.txt", "r"); // 以读方式打开文本
//     if(fp == NULL)
//     {
//         perror("fopen");
//         return 1;
//     }

//     int count = 0;
//     char buffer[64];
//     while(count < 10)
//     {
//         fgets(buffer, 64, fp);
//         printf("%s\n", buffer);
//         count++;
//     }

//     fclose(fp);
//     return 0;
// }

// #include<stdio.h>
// #include<string.h>

// int main()
// {
//     FILE* fp = fopen("log.txt", "a");//以追加的打开当前目录下的log.txt文件,没有就新建,如果目标文件存在,a写时不会清空目标文件,在文件内容最后写入
//     if(fp == NULL)
//     {
//         perror("fopen");
//         return 1;
//     }

//     const char* msg = "Hello linux\n";
//     //fwrite(msg, strlen(msg) + 1, 1, fp); // 乱码
//     fwrite(msg, strlen(msg), 1, fp);                                                                               

//     fclose(fp);
//     return 0;
// }


// #include <stdio.h>

// // 用不同的比特位来表示不同的信号
// #define ONE (1 << 0) // 0
// #define TWO (1 << 1) // 1
// #define THREE (1 << 2) // 2
// #define FOUR (1 << 3)  // 4
// void showSig(int flags)
// {
//     if(flags & ONE) 
//         printf("one\n");    
//     else if(flags & TWO) 
//         printf("two\n");    
//     else if(flags & THREE) 
//         printf("three\n");    
//     else if(flags & FOUR) 
//         printf("four\n");
// }
// int main()
// {
//     showSig(ONE);    
//     printf("------------------------\n");   
//     showSig(TWO);    
//     printf("------------------------\n");    
//     showSig(ONE|TWO);    
//     printf("------------------------\n");    
//     showSig(ONE|TWO|THREE);    
//     printf("------------------------\n");    
//     showSig(ONE|TWO|THREE|FOUR);    
//     printf("------------------------\n");

//     return 0;
// }

// #include <stdio.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// int main()
// {
//     int fd1 = open("log1.txt", O_WRONLY | O_CREAT, 0664);
//     if(fd1 < 0)
//     {
//         perror("open");
//         return 1;
//     }

//     int fd2 = open("log2.txt", O_WRONLY | O_CREAT, 0644);
//     if(fd2 < 0)
//     {
//         perror("open");
//         return 1;
//     }

//     // 关闭文件
//     close(fd1);
//     close(fd2);
//     return 0;
// }

// #include <stdio.h>
// #include <string.h>
// #include <unistd.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// int main()
// {
//     int fd = open("log.txt", O_WRONLY | O_CREAT | O_APPEND, 0666);
//     if(fd < 0)
//     {
//         perror("open");
//         return 1;
//     }

//     // 将字符串写到文件中
//     char buffer[1024];
//     int cnt = 5;
//     while(cnt--)
//     {
//         // 将字符串和数字通过sprintf转化为字符串
//         sprintf(buffer, "%s：%d\n", "after append!!!", cnt + 10);

//         // 将新内容写入文件，注意系统接口是不包括'\0'的大小的
//         write(fd, buffer, strlen(buffer));
//     }

//     // 关闭文件
//     close(fd);
//     return 0;
// }


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
int main()
{
    int fd = open("log.txt", O_RDONLY);
    if(fd < 0)
    {
        perror("open");
        return 1;
    }

    // 读取文件中的信息
    char buffer[1024];
    // 这里sizeof - 1是为了留位置给我们自己添加的'\0'腾出位置，因为文件中是不含'\0'的，要多预留一个位置出来
    ssize_t num = read(fd, buffer, sizeof(buffer) - 1);

    // 大于0表示读取成功，并将buffer的末尾设为'\0'，因为系统接口是不为我们添加'\0'的
    if(num != 0)
    {
        buffer[num] = '\0';
        printf("%s", buffer);
    }

    // 关闭文件
    close(fd);
    return 0;
}