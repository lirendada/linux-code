#pragma once
#include <iostream>

enum
{
    USAGE_ERR = 1,
    SOCKET_ERR,
    BIND_ERR,
    LISTEN_ERR,
    ACCEPT_ERR,
    EPOLL_CREATE_ERR,
    EPOLL_CTL_ERR,
    NEW_EVENTS_ERR
};