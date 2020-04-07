//
// Created by jmq on 2020/4/7.
//

#include "client.h"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <cerrno>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace std;

Client::Client(const char *serverIp, int port) {
    bzero(&serverAddr_, sizeof(serverAddr_));
    serverAddr_.sin_family = PF_INET;
    serverAddr_.sin_port = htons(port);
    serverAddr_.sin_addr.s_addr = inet_addr(serverIp);

    sock_ = socket(PF_INET, SOCK_STREAM, 0);
    if (sock_ < 0) {
        perror("create sock error");
        exit(-1);
    }

    epFd_ = epoll_create(1000);
    if (epFd_ < 0) {
        perror("epoll_create error");
        exit(-1);
    }
}



void Client::Start() {

    if (connect(sock_, (struct sockaddr *)&serverAddr_, sizeof(serverAddr_)) < 0) {
        perror("connect error");
        exit(-1);
    }

    /*
     *  子进程: 1.等待用户输入信息 2. 将聊天信息写入到管道pipe中，并发送给父进程
     *  父进程 1.使用epoll机制接受服务端发来的信息，并显示给用户 2.将子进程发的信息从管道中读出，并发送给服务器
     */

    //pipeFd[0] 父进程读， pipeFd[1] 子进程写
    int pipeFd[2];
    if (pipe(pipeFd) < 0) {
        perror("create pipe error");
        exit(-1);
    }

    struct epoll_event events[2];
    AddFd(sock_);

    // 有数据读时，通知
    AddFd(pipeFd[0]);

    char message[BUF_SIZE] = {0};
    //
    bool run = true;

    int pid = fork();
    if (pid < 0) {
        perror("fork error");
        exit(-1);
    } else if (pid == 0) {
        //子进程：1.等待用户输入信息 2. 将聊天信息写入到管道pipe中，并发送给父进程
        close(pipeFd[0]);
        cout<<"Enter string to send  "<<endl;

        while(run) {
            bzero(message, BUF_SIZE);
            cin.getline(message, BUF_SIZE);

            // 将消息写入管道
            if (write(pipeFd[1], message, strlen(message)) < 0) {
                perror("write error");
                exit(-1);
            }
            if (!strcmp(message, "exit")) {
                sleep(1);
                break;
            }
        }
    } else {
        //父进程：1.使用epoll机制接受服务端发来的信息，并显示给用户 2.将子进程发的信息从管道中读出，并发送给服务器
        close(pipeFd[1]);
        while (run) {
            int readyNum = epoll_wait(epFd_, events, 2, -1);
            if (readyNum < 0) {
                perror("epoll_wait error");
                exit(-1);
            }

            for (int i = 0; i < readyNum; i++) {
                bzero(message, BUF_SIZE);

                if (events[i].data.fd == sock_) {
                    // 服务器发来的消息
                    int ret = recv(sock_, message, BUF_SIZE, 0);

                    if (ret == 0) {
                        //服务端关闭
                        close(sock_);
                        run = false;
                    } else {
                        cout << string(message) << endl;
                    }
                } else {
                    //来自子进程的消息
                    int ret = read(events[i].data.fd, message, BUF_SIZE);
                    if (ret == 0) {
                        run = false;
                    } else {

                        if (send(sock_, message, strlen(message), 0) < 0) {
                            perror("send error");
                            exit(-1);
                        }
                        if (!strcmp(message, "exit")) {
                            sleep(1);
                            return;
                        }
                    }
                }
            }
        }
    }
}


/*
 * enable : 决定epoll是LT或者ET模式
 */
void Client::AddFd( int fd) {
    struct epoll_event  ev;
    ev.data.fd = fd;
    // true : 边缘模式， 一次读不完下次不在读直到有消息再来

    bool enable = true;
    ev.events = EPOLLIN;
    if (enable) {
        ev.events = EPOLLIN | EPOLLET;
    }
    if (epoll_ctl(epFd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
        perror("epoll_ctl Error");
        exit(-1);
    }

    //设置socket为非阻塞
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
    cout<< fd << " 加入到 EPOLL中" << endl;
}

Client::~Client() {
    close(sock_);
}
