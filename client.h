//
// Created by jmq on 2020/4/7.
//

#ifndef CLIENT_CLIENT_H
#define CLIENT_CLIENT_H

#include <string>
#include <netinet/in.h>
using namespace std;

#define  BUF_SIZE 2048

class Client {
public:

    Client(const char *serverIp, int port);

    void Start();

    void AddFd(int fd);

    ~Client();

private:
    //和服务器通信的sock
    int sock_;

    //epoll句柄
    int epFd_;

    //服务器地址serverAddr信息
    struct sockaddr_in serverAddr_;

};


#endif //CLIENT_CLIENT_H
