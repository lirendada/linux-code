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
    EPOLL_WAIT_ERR,
    NEW_EVENTS_ERR,
    GET_NON_BLOCKING_ERR,
    SET_NON_BLOCKING_ERR,
    NEW_CONNECTION_ERR,
    OK,
    DIV_ZERO,
    MOD_ZERO,
    OP_ERROR
};