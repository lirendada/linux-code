#include "echoserver.hpp"

int main()
{
    EchoServer echoserver(8080);
    echoserver.start();
    return 0;
}