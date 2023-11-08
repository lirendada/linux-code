#include "httpserver.hpp"

const std::string source = "./wwwroot";

std::string RequestStr(const HttpRequest &req) 
{
    std::stringstream ss;
    ss << req._method << " " << req._path << " " << req._version << "\r\n";
    for (auto &it : req._queryString) 
        ss << it.first << ": " << it.second << "\r\n";
    for (auto &it : req._header) 
        ss << it.first << ": " << it.second << "\r\n";
    ss << "\r\n" << req._body;
    return ss.str();
}

void Hello(const HttpRequest &req, HttpResponse *rsp) 
{
    rsp->set_content(RequestStr(req), "text/plain");
    // sleep(15);
}

void Login(const HttpRequest &req, HttpResponse *rsp) 
{
    rsp->set_content(RequestStr(req), "text/plain");
}

void PutFile(const HttpRequest &req, HttpResponse *rsp) 
{
    std::string pathname = source + req._path;
    Util::write_file(pathname, req._body);
}

void DelFile(const HttpRequest &req, HttpResponse *rsp) 
{
    rsp->set_content(RequestStr(req), "text/plain");
}

int main()
{
    HttpServer server(8080, 10);
    server.set_nums_of_thread(4);
    server.set_static_directory(source);
    server.add_get("/hello", Hello);
    server.add_post("/login", Login);
    server.add_put("/1234.txt", PutFile);
    server.add_delete("/1234.txt", DelFile);
    server.start_httpserver();
    return 0;
}