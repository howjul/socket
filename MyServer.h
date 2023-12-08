#ifndef MY_SERVER_H
#define MY_SERVER_H

#include<iostream>
#include<string>
#include<map>
#include<sstream>

#include<time.h>
#include<pthread.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/un.h>
#include<netdb.h>
#include<unistd.h>

#include"MyPacket.h"

#define PORT 3283 //服务器端口号
#define MAX_BACKLOG 5 //请求连接队列的最大长度
#define MAX_CLIENT 64 //最大客户端连接数
#define MAX_BUFFER 1024 //首发数据缓冲区大小

using namespace std;

struct client_info{
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
    string get_list();
    int find_in_list(int id);
    void set_client_list(int list_num);
    void rst_client_list(int list_num);
};

struct thread_info{
  int list_num; //客户端列表的序号
  int client_sockfd; //客户端套接字
  sockaddr_in client_addr; //客户端地址
  MyServer* server; 
};

void* handle_client(void* thread_info); //处理客户端请求
int handle_request(MyPacket request, struct thread_info info); //处理单个请求
void type_t(int client_sockfd);
void type_n(int client_sockfd);
void type_d(int client_sockfd);
void type_e(int client_sockfd);
void type_h(int client_sockfd, sockaddr_in client_addr);
void type_a(int client_sockfd, string message);
void type_l(int client_sockfd, MyServer server);
void type_s(int client_sockfd, char targetid, string message, MyServer server);

#endif