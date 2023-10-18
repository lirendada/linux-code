// #include <any>
// #include <iostream>

// int main()
// {
//     std::any a = 1;
//     std::cout << a.type().name() << ": " << std::any_cast<int>(a) << std::endl;

//     a = 3.14;
//     std::cout << a.type().name() << ": " << std::any_cast<double>(a) << std::endl;

//     a = true;
//     std::cout << std::boolalpha << a.type().name() << ": " << std::any_cast<bool>(a) << std::endl;

//     // 有误的转型
//     try
//     {
//         a = 1;
//         std::cout << std::any_cast<float>(a) << std::endl;
//     }
//     catch (const std::bad_any_cast& e)
//     {
//         std::cout << e.what() << '\n';
//     }

//     // 拥有值
//     a = 1;
//     if(a.has_value())
//         std::cout << a.type().name() << std::endl;
    
//     // 重置
//     a.reset();
//     if (!a.has_value())
//         std::cout << "no value" << std::endl;
 
//     // 指向所含数据的指针，对变量取地址使用
//     a = 1;
//     int* i = std::any_cast<int>(&a);
//     std::cout << *i << std::endl;
//     return 0;
// }

#include <iostream>
#include <typeinfo>
#include <string>

class Any
{
private:
    class holder
    {
    public:
        virtual ~holder() {}               // 析构函数，父类需要设为虚函数才能正确释放子类
        virtual const std::type_info& type() = 0; 
        virtual holder* clone() = 0;      
    };

    template <class T>
    class placeholder : public holder
    {
    public:
        placeholder(const T& val) : _val(val) 
        {}

        // 用于返回子类中持有的数据类型
        virtual const std::type_info& type() { return typeid(T); }

        // 针对当前的对象自身，克隆出一个新的子类对象
        virtual holder* clone() { return new placeholder<T>(_val); }       

        T _val; // 任意类型的数据
    };

    holder* _content; // holder类对象，通过多态方式来操作placeholder对象
public:
    Any() 
        : _content(nullptr) 
    {}

    ~Any() { delete _content; }

    // 任意类型数据的构造函数
    template <class T>
    Any(const T& val) 
        : _content(new placeholder<T>(val))  
    {}   

    // Any类型的构造函数
    Any(const Any& other) 
    { 
        if(other._content == nullptr)
            _content = nullptr;
        _content = other._content->clone();
    }

    // 任意类型数据的赋值重载函数
    template <class T>
    Any& operator=(const T& val)
    {
        // 为val构造一个临时的通用容器，然后与当前容器自身进行指针交换，临时对象释放的时候，原先保存的数据也就被释放
        Any(val).swap(*this);
        return *this;
    }

    // Any类型的赋值重载函数
    Any& operator=(const Any& other)
    {
        Any(other).swap(*this);
        return *this;
    }

    // 返回placeholder对象保存的数据的指针
    template <class T>
    T* get()
    {
        if(_content->type() != typeid(T))
            return nullptr;
        return &((placeholder<T>*)_content)->_val;
    }

    const std::type_info& type() { return _content->type(); }
private:
    Any& swap(Any& other)
    {
        std::swap(_content, other._content);
        return *this;
    }
};

class Test
{
public:
    Test() {std::cout << "构造" << std::endl;}
    Test(const Test &t) {std::cout << "拷贝" << std::endl;}
    ~Test() {std::cout << "析构" << std::endl;}
};

int main()
{
    Any a;
    a = 10;
    int* pa = a.get<int>();
    std::cout << *pa << std::endl;
    std::cout << a.type().name() << std::endl;

    a = std::string("lirendada");
    std::string* sa = a.get<std::string>();
    std::cout << *sa << std::endl;
    std::cout << a.type().name() << std::endl;

    a = Test();
    Test* ta = a.get<Test>();
    std::cout << a.type().name() << std::endl;
    
    return 0;
}