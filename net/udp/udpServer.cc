#include "udpServer.hpp"
#include <memory>
#include <unordered_map>
#include <fstream>
#include <signal.h>
#include <stdio.h>
using namespace std;
using namespace Server;

static void Usage(string proc)
{
    cout << "\nUsage:\n\t" << proc << " local_port\n\n";
}

static const string dictpath = "./dict.txt"; // 单词文件的路径
static unordered_map<string, string> dict;   // 存放单词的容器

static void initdict(char Separator)
{
    ifstream in(dictpath, ios::binary);
    if(!in.is_open()) 
    {
        cerr << "open file " << dictpath << " error" << endl; 
        exit(OPEN_ERR);
    }

    // 按行获取文件中的单词，比如apple:苹果，注意下面使用的substr是左闭右开的
    string line;
    while(getline(in, line))
    {
        size_t pos = line.find(Separator);
        if(pos == string::npos)
        {
            cerr << "未找到对应单词的翻译" << endl;
            exit(NOTFOUND_ERR);
        }
        dict[line.substr(0, pos)] = line.substr(pos + 1); // 找到了就写到字典中
    }
    in.close();

    cout << "load dict success" << endl;
}

// 测试单词是否成功读取的测试函数
static void debugPrint()
{
    for(auto &e : dict)
    {
        cout << e.first << " : " << e.second << endl;
    }
}

// demo1 -- 简单的中英文翻译
void handlerMessage1(int sockfd, string clientip, uint16_t clientport, string message)
{
    // 就可以对message进行特定的业务处理，而不关心message怎么来的 ---- server通信和业务逻辑解耦！
    // 婴儿版的业务逻辑
    string response_message;
    auto iter = dict.find(message);
    if(iter == dict.end())
        response_message = "unknown!";
    else
        response_message = iter->second;

    // 发回给客户端
    struct sockaddr_in client;
    bzero(&client, 0);
    client.sin_family = AF_INET;
    client.sin_port = htons(clientport);
    client.sin_addr.s_addr = inet_addr(clientip.c_str());

    ssize_t n = sendto(sockfd, response_message.c_str(), response_message.size(), 0, (struct sockaddr*)&client, sizeof client);
    if(n == -1)
    {
        cerr << "send error: " << errno << " : " << strerror(errno) << endl; 
        exit(SEND_ERR);
    }
}

// 自定义信号，达到热加载目的
static void reload(int signo)
{
    (void)signo;
    initdict(':');
}

// demo2 -- 远程命令行指令解析
void handlerMessage2(int sockfd, string clientip, uint16_t clientport, string cmd)
{
    // 先排除一些有风险的指令
    if(cmd.find("rm") != string::npos
        || cmd.find("mv") != string::npos
        || cmd.find("cp") != string::npos
        || cmd.find("rmdir") != string::npos
        || cmd.find("while") != string::npos)
    {
        cerr << clientip << ": " << clientport << " 正在做一个非法的操作: " << cmd << endl;
        return;
    }

    // popen = pipe + fork + exec*
    FILE* stream = popen(cmd.c_str(), "r");
    string response;
    if(stream == nullptr)
        response = cmd + " exec failed";

    char line[1024];
    while(fgets(line, sizeof(line), stream))
    {
        response += line; // 按行读取
    }

    // 发回给客户端（这部分都是一致的，无需修改）
    struct sockaddr_in client;
    bzero(&client, 0);
    client.sin_family = AF_INET;
    client.sin_port = htons(clientport);
    client.sin_addr.s_addr = inet_addr(clientip.c_str());

    ssize_t n = sendto(sockfd, response.c_str(), response.size(), 0, (struct sockaddr*)&client, sizeof client);
    if(n == -1)
    {
        cerr << "send error: " << errno << " : " << strerror(errno) << endl; 
        exit(SEND_ERR);
    }

    pclose(stream); // 使用这个而不是fclose来关闭
}


// demo3 -- 一个简易的小聊天室
onlineUser users;

void handlerMessage3(int sockfd, string clientip, uint16_t clientport, string cmd)
{
    // 判断是否为上下线请求
    if(cmd == "online")
        users.addOnlineUser(clientip, clientport);
    else if(cmd == "offline")
        users.delOnlineUser(clientip, clientport);

    // 判断是否在线，是的话则进行信息的广播，不是的话则提示请登录
    if(users.isOnlineUser(clientip, clientport) == true)
    {
        // 消息的广播
        users.broadcastMessage(sockfd, clientip, clientport, cmd);
    }
    else
    {
        // 单独提示需要上线
        struct sockaddr_in client;
        bzero(&client, sizeof(client));
        client.sin_family = AF_INET;
        client.sin_port = htons(clientport);
        client.sin_addr.s_addr = inet_addr(clientip.c_str());

        string response = "你还没有上线，请先上线，运行: online";
        sendto(sockfd, response.c_str(), response.size(), 0, (struct sockaddr*)&client, sizeof client);
    }
}

int main(int argc, char* argv[])
{
    if(argc != 2) // 如果参数传递不为两个，则提醒使用者，并且退出程序
    {
        Usage(argv[0]);
        exit(USAGE_ERR);
    }
    uint16_t port = atoi(argv[1]); // 将端口号类型转换为uint16_t
    
    // 热加载功能，也就是我们不需要退出程序进行字典的更新
    // 只需要捕捉2号信号，让它帮助我们去重新调用一次initdict()函数即可！
    // signal(2, reload); 
    // initdict(':');
    // debugPrint();

    unique_ptr<udpServer> udper(new udpServer(handlerMessage3, port)); // 创建服务端对象
    udper->initServer();
    udper->start();

    return 0;
}