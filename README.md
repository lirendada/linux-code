# linux-code

Linux 系统编程与网络编程的学习代码（C / C++，CentOS 7 环境）。从进程、管道、信号、共享内存到多线程与线程池，再到套接字、自定义协议、HTTP 服务器和多路转接，每个目录对应一个专题，可独立编译运行。

## 目录索引

| 目录 | 内容 | 语言 / 工具 |
|---|---|---|
| `firstCode/` | 最初的练习代码与 shell 实验文件 | C |
| `process/` | 进程创建与控制 | C |
| `playcode/` | 系统编程专题：缓冲区、文件系统、环境变量、动态库与静态库的制作与使用、模拟实现 stdio、进程替换、进程地址空间、简易 shell | C / C++ |
| `pipe/` | 匿名管道、命名管道、基于管道的进程池 | C++ |
| `shm/` | 共享内存的 server / client 通信 | C++ |
| `signal/` | 信号的捕捉、阻塞与处理 | C++ |
| `pthread/` | 线程封装、阻塞队列、环形队列、线程池 | C++ |
| `net/` | UDP / TCP 套接字、自定义应用层协议（`protocol/`）、HTTP 服务器（`http/`）、Windows 端 UDP 客户端测试 | C++ |
| `advanced_IO/` | 非阻塞 IO、select / poll / epoll 多路转接服务器、Reactor 模式 | C++ |
| `protobuf_learning/` | Protobuf 快速上手与 proto3 语法 | C++ / protoc |
| `mysql_test/` | MySQL C API 连接测试 | C++ |
| `algorithm/` | KMP 字符串匹配 | C++ |
| `designmode/` | 设计模式示例 | C++ |
| `vector/` | 手写 vector | C++ |
| `test_makefile/` | 多文件工程的 Makefile 练习 | C / Makefile |
| `cpp_test/` | 零散的 C++ 语法测试 | C++ |

## 约定

- 每个专题目录自带 Makefile，进入目录执行 `make` 即可生成同名可执行文件，`make clean` 清理；统一使用 `g++ -std=c++11`，多线程程序链接 `-lpthread`。
- 部分目录连同编译产物一起提交，方便直接查看运行结果；重新编译会覆盖。
- `Centos-7.repo` 是当时环境的软件源配置，仅作环境记录。
