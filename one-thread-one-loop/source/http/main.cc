#include "httpserver.hpp"

int main()
{
    std::cout << Util::is_path_valid("/../server.hpp") << std::endl;
    std::cout << Util::is_path_valid("/img/../main.cc") << std::endl;
    std::cout << Util::is_path_valid("/img/../../main.cc") << std::endl;
    return 0;
}