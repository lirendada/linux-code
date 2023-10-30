#include "httpserver.hpp"

int main()
{
    std::cout << Util::url_decode("c++", true) << std::endl;
    std::cout << Util::url_decode("c++", false) << std::endl << std::endl;

    std::cout << Util::url_decode("c%20%20", true) << std::endl;
    std::cout << Util::url_decode("c%20%20", false) << std::endl << std::endl;

    std::cout << Util::url_decode("%2Flogin%2Fuser%3Dlirendada+%26+id+%3D+2", false) << std::endl;
    std::cout << Util::url_decode("%2Flogin%2Fuser%3Dlirendada%20%26%20id%20%3D%202", false) << std::endl;
    return 0;
}