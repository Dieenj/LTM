#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <vector>
#include <map>
#include <netinet/in.h>

// Trạng thái của một Client đang kết nối
struct ClientSession {
    int socketFd;
    std::string username;
    bool isAuthenticated;
    std::string currentDir;

    ClientSession() : socketFd(-1), isAuthenticated(false), currentDir("/") {}
};

#endif // SERVER_H