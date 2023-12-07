#ifndef MY_SERVER_H
#define MY_SERVER_H

#include<iostream>
#include<string>
#include<map>

#include<pthread.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/un.h>
#include<netdb.h>
#include<unistd.h>

#define PORT 3283 //服务器端口号
#define MAX_BACKLOG 5 //请求连接队列的最大长度
#define MAX_CLIENT 64 //最大客户端连接数

using namespace std;

struct client_info{
  int client_sockfd; //客户端套接字
  sockaddr_in client_addr; //客户端地址
};

struct thread_info{
  int list_num; //客户端列表的序号
  int client_sockfd; //客户端套接字
  sockaddr_in client_addr; //客户端地址
};

class MyServer{
  private:
    int server_sockfd; //服务器套接字
    sockaddr_in server_addr; //服务器地址
    map<int, struct client_info> client_list; //客户端列表
    int client_list_situation[MAX_CLIENT]; //客户端列表的占用情况
  public:
    MyServer(); //构造函数，初始化服务器
    ~MyServer(); //析构函数
    void on(); //启动服务器
    int get_list_num(); //获得一个list的序号
};

void* handle_client(void* thread_info); //处理客户端请求

#endif