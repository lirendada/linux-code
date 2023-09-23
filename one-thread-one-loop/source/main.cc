#include "server.hpp"

int main()
{
    Buffer buf;

/*
    // 测试读取一行数据问题
    for(int i = 0; i < 300; ++i)
    {
        std::string str = "lirendada tthh!" + std::to_string(i) + '\n';
        buf.write_string_andMove(str);
    }
    while(buf.get_sizeof_read() > 0)
        std::cout << buf.get_line_andMove() << std::endl;
*/

/*
    // 测试扩容问题
    for(int i = 0; i < 300; ++i)
    {
        std::string str = "lirendada tthh!" + std::to_string(i) + '\n';
        buf.write_string_andMove(str);
    }
    std::string tmp;
    tmp = buf.read_to_string_andMove(buf.get_sizeof_read());
    std::cout << tmp << std::endl;
*/

/*
    // 测试简单的读写操作
    buf.write_string_andMove(str); // 进行写入
    Buffer buf1;
    buf1.write_Buffer_andMove(buf);

    // 对buf进行读取
    std::string tmp;
    tmp = buf.read_to_string_andMove(buf.get_sizeof_read());

    std::cout << tmp << std::endl;
    std::cout << buf.get_sizeof_read() << std::endl;
    std::cout << buf1.get_sizeof_read() << std::endl;

    buf.write_data_andMove("hha", 3);
    std::cout << buf.get_sizeof_read() << std::endl;
*/
    return 0;
}