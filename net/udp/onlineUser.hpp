#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
using namespace std;

class User
{
public:
    User(const string& ip, uint16_t port) : _ip(ip), _port(port)
    {}
    ~User()
    {}
    string getip() { return _ip; }
    uint16_t getport() { return _port; }
private:
    string _ip;
    uint16_t _port;
};

class onlineUser
{
public:
    onlineUser() {}
    ~onlineUser() {}

    void addOnlineUser(const string& ip, const uint16_t& port)
    {
        string id = ip + "-" + to_string(port);
        _users[id] = User(ip, port);
    }
    void delOnlineUser(const string& ip, const uint16_t& port)
    {
        string id = ip + "-" + to_string(port);
        _users.erase(id);
    }
    bool isOnlineUser(const string& ip, const uint16_t& port)
    {
        string id = ip + "-" + to_string(port);
        return _users.find(id) == _users.end() ? false : true;
    }
    void broadcastMessage()
    {
        
    }
private:
    unordered_map<string, User> _users;
};