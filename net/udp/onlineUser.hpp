#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
using namespace std;

class User
{
public:
    User(const string& ip, uint16_t port) : _ip(ip), _port(port)
    {}
    ~User()
    {}
    string& getip() { return _ip; }
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

    void addOnlineUser(const string& ip, uint16_t port)
    {
        string id = ip + "-" + to_string(port);
        _users.insert(make_pair(id, User(ip, port)));
    }
    void delOnlineUser(const string& ip, uint16_t port)
    {
        string id = ip + "-" + to_string(port);
        _users.erase(id);
    }
    bool isOnlineUser(const string& ip, uint16_t port)
    {
        string id = ip + "-" + to_string(port);
        return _users.find(id) == _users.end() ? false : true;
    }
    void broadcastMessage(int sockfd, const string& ip, uint16_t port, const string& message)
    {
        for(auto& user: _users)
        {
            string id = ip + "-" + to_string(port) + "# " + message;

            struct sockaddr_in client;
            bzero(&client, sizeof client);
            client.sin_family = AF_INET;
            client.sin_port = htons(user.second.getport());
            client.sin_addr.s_addr = inet_addr(user.second.getip().c_str());
            sendto(sockfd, id.c_str(), id.size(), 0, (struct sockaddr*)&client, sizeof(client));
        }
    }
private:
    unordered_map<string, User> _users;
};