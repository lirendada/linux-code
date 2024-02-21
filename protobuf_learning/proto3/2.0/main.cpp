#include <iostream>
#include <fstream>
#include <string>
#include "contacts.pb.h"
using namespace std;

void add_new_people(contacts2::PeopleInfo* people)
{
    cout << "-------新增联系人-------" << endl;

    cout << "输入联系人姓名：";
    string name;
    getline(cin, name);
    people->set_name(name);

    cout << "输入联系人年龄：";
    int age = 0;
    cin >> age;
    people->set_age(age);

    // 要先将之前回车键去掉
    cin.ignore(256, '\n');
    while(true)
    {
        cout << "输入联系人电话（按回车停止输入）：";
        string number;
        getline(cin, number);
        if(number.empty())
            break; // 空字符串则直接退出去

        contacts2::PeopleInfo_Phone* phone = people->add_phone();
        phone->set_number(number);
    }

    cout << "--------新增成功--------" << endl;
}

int main()
{
    /* GOOGLE_PROTOBUF_VERIFY_VERSION 宏: 验证没有意外链接到与编译的头⽂件不兼容的库版本。
    如果检测到版本不匹配，程序将中⽌。注意，每个 .pb.cc ⽂件在启动时都会⾃动调⽤此宏。
    在使⽤ C++ Protocol Buffer 库之前执⾏此宏是⼀种很好的做法，但不是绝对必要的。*/
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    // 1. 读取本地通讯录，并且通过contact进行解析二进制流
    contacts2::Contacts contact;          // 注意使用消息类的时候需要指明命名空间！！！
    fstream in("contact.bin", ios::in | ios::binary);
    if(in.fail()) 
    {
        cerr << "不存在本地通讯录（也可能是读取错误），创建新的本地通讯录！" << endl;
    }   
    else if(contact.ParseFromIstream(&in) == false)
    {
        cerr << "本地通讯录二进制流解析失败" << endl;
        in.close();
        return -1;
    }

    // 2. 写入新的联系人
    add_new_people(contact.add_people());

    // 3. 将通讯录写入本地文件中
    fstream out("contact.bin", ios::out | ios::binary | ios::trunc);
    if(contact.SerializeToOstream(&out) == false) 
    {
        cerr << "写入本地通讯录失败！" << endl;
        in.close();
        out.close();
        return -1;
    }
    in.close();
    out.close();

    /* 在程序结束时调⽤ ShutdownProtobufLibrary()，为了删除 Protocol Buffer 库分配的所
    有全局对象。对于⼤多数程序来说这是不必要的，因为该过程⽆论如何都要退出，并且操作系统将负责
    回收其所有内存。但是，如果你使⽤了内存泄漏检查程序，该程序需要释放每个最后对象，或者你正在
    编写可以由单个进程多次加载和卸载的库，那么你可能希望强制使⽤ Protocol Buffers 来清理所有内容。*/
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
