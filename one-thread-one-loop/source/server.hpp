#pragma once
#include <iostream>
#include <vector>
#include <cassert>
#include <cstring>

const static uint64_t MAX_BUFFER_SIZE = 1024;

class Buffer
{
private:
    std::vector<char> _buffer; // 缓冲区
    uint64_t _reader;	   // 当前读位置
    uint64_t _writer;      // 当前写位置
public:
    Buffer() :_reader(0), _writer(0), _buffer(MAX_BUFFER_SIZE) {}
    
    /////////////////////////////  写操作 /////////////////////////////
    // 获取当前写入的起始地址
    char* start_of_write()
    {
        return &(*_buffer.begin()) + _writer;
    }

    // 获取可写数据的大小
    uint64_t get_sizeof_write()
    {
        // 即开头空闲空间 + 结尾空闲空间
        return get_head_free_size() + get_tail_free_size();
    }

    // 将写位置向后移动
    void push_writer_back(uint64_t size)
    {
        // 向后移动的大小，必须小于当前后边的空闲空间大小
        assert(size <= get_tail_free_size());
        _writer += size;
    }

    // 写入数据操作（将data中数据写到缓冲区中）
    void write_data(const void* data, uint64_t size)
    {
        // 1. 确保空间足够
        if(size == 0)
            return;
        ensure_write_space(size);

        // 2. 将数据拷贝到缓冲区，注意这里需要转换类型
        const char* tmp = (const char*)data;
        std::copy(tmp, tmp + size, start_of_write());
    }

    // 写入数据操作，并让写入位置向后移动
    void write_data_andMove(const void* data, uint64_t size)
    {
        write_data(data, size);
        push_writer_back(size);
    }

    // 写入string类型数据操作
    void write_string(const std::string& data)
    {
        return write_data(data.c_str(), data.size());
    }

    // 写入string类型数据操作，并让写入位置向后移动
    void write_string_andMove(const std::string& data)
    {
        write_string(data);
        push_writer_back(data.size());
    }

    // 写入Buffer类型数据操作
    void write_Buffer(Buffer& data)
    {
        return write_data(data.start_of_read(), data.get_sizeof_read());
    }

    // 写入Buffer类型数据操作，并让写入位置向后移动
    void write_Buffer_andMove(Buffer& data)
    {
        write_Buffer(data);
        push_writer_back(data.get_sizeof_read());
    }

    // 确保可写空间足够大（整体空闲空间够了就移动数据，否则就扩容）
    void ensure_write_space(uint64_t size)
    {
        // 如果末尾空闲空间大小足够，直接返回
        if(size <= get_tail_free_size())
            return;

        // 末尾空闲空间不够，则判断加上起始位置的空闲空间大小是否足够, 够了就将数据移动到起始位置
        if(size <= get_sizeof_write())
        {
            // 1. 先记录下可读数据的大小，防止下面搬移后变了
            uint64_t read_size = get_sizeof_read();

            // 2. 将可读部分的数据都搬到数组开头
            std::copy(start_of_read(), start_of_read() + read_size, &(*_buffer.begin()));
            
            // 3. 然后重新设置读写位置
            _reader = 0;
            _writer = read_size;
        }
        else
        {
            // 总体空闲空间不够，则需要扩容，不移动数据，直接在写偏移之后扩容足够空间即可
            _buffer.resize(_writer + size);
        }
    }

    /////////////////////////////  读操作 /////////////////////////////
    // 获取当前读取的起始地址
    char* start_of_read()
    {
        return &(*_buffer.begin()) + _reader;
    }

    // 获取可读数据的大小
    uint64_t get_sizeof_read()
    {
        // 可读数据的大小 = 写偏移 - 读偏移
        return _writer - _reader;
    }

    // 将读位置向后移动
    void push_reader_back(uint64_t size)
    {
        // 向后移动的大小，必须小于可读数据大小
        assert(size <= get_sizeof_read());
        _reader += size;
    }

    // 读取数据操作（读取到buffer中）
    void read_data(char* buffer, uint64_t size)
    {
        // 1. 要求要获取的数据大小必须小于可读数据大小
        assert(size <= get_sizeof_read());

        // 2. 读取数据到buffer中
        std::copy(start_of_read(), start_of_read() + size, (char*)buffer);
    }

    // 读取数据操作（读取到buffer中），并将读取位置向后移动
    void read_data_andMove(char* buffer, uint64_t size)
    {
        read_data(buffer, size);
        push_reader_back(size);
    }

    // 读取数据后转化为string类型返回
    std::string read_to_string(uint64_t size)
    {
        // 1. 要求要获取的数据大小必须小于可读数据大小
        assert(size <= get_sizeof_read());

        // 2. 调用read_data接口进行读写
        std::string str;
        str.resize(size);
        read_data(&str[0], size); // 传参时候不能使用c_str()，因为其类型是const的
        return str;
    }

    // 读取数据后转化为string类型返回，并且让读取位置向后移动
    std::string read_to_string_andMove(uint64_t size)
    {
        std::string str = read_to_string(size);
        push_reader_back(size);
        return str;
    }

    /////////////////////////////  其它操作 /////////////////////////////
    // 清空缓冲区接口
    void clear_buffer()
    {
        // 只需要将偏移量置零即可
        _writer = _reader = 0;
    }

    // 找到换行的位置（方便http解析）
    char* find_CRLF()
    {
        // 这里我们使用memchr找到\n就算找到换行位置
        char* res = (char*)memchr(start_of_read(), '\n', get_sizeof_read());
        return res;
    }
    
    // 获取一行数据（方便http解析）
    std::string get_line()
    {
        char* pos = find_CRLF();
        if(pos == nullptr)
            return "";
        
        // 这里+1是为了把换行字符也取出来
        return read_to_string(pos - start_of_read() + 1);
    }

    // 获取一行数据然后移动读取下标
    std::string get_line_andMove()
    {
        std::string str = get_line();
        push_reader_back(str.size());
        return str;
    }
private:
    // 获取缓冲区末尾空闲空间的大小 -- 即写位置之后的空闲空间
    uint64_t get_tail_free_size()
    {
        return _buffer.size() - _writer;
    }

    // 获取缓冲区开头空闲空间的大小 -- 即读位置之前的空闲空间
    uint64_t get_head_free_size()
    {
        return _reader;
    }
};