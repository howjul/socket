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
#include<chrono>
#include<mutex>

#include"MyPacket.h"

#define PORT 13283 //服务器端口号
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
    bool _is_end = false; //用于指示server是否已经接待过客户端，若接待过且有一个客户端退出则为true
  public:
    MyServer(); //构造函数，初始化服务器
    ~MyServer(); //析构函数
    void on(); //启动服务器

    // 接口函数
    void set_client_list(int list_num);
    void rst_client_list(int list_num);
    string get_list();
    int get_server_sockfd();
    void set_is_end_true();
    bool is_end(); 

    // 辅助函数
    int get_list_num(); //获得一个客户端列表的序号
    int find_in_list(int id); //通过id查找客户端套接字
    bool check_client_list(); //检查客户端列表是否为空
};

struct thread_info{
  int list_num; //客户端列表的序号
  int client_sockfd; //客户端套接字
  sockaddr_in client_addr; //客户端地址
  MyServer* server; //服务器
};

MyServer primary_server; //定义主服务器

void* handle_client(void* thread_info); //处理客户端请求
int handle_request(MyPacket request, struct thread_info info, int request_id); //处理单个请求

//具体请求处理函数
void type_h(int client_sockfd, sockaddr_in client_addr); //成功连接响应
void type_t(int client_sockfd); //时间请求
void type_n(int client_sockfd); //名字请求
void type_l(int client_sockfd, MyServer server); //列表读取请求
void type_s(int client_sockfd, int client_id, char targetid, string message, MyServer server); //转发请求
void type_a(int client_sockfd, string message); //转发确认回复
void type_d(int client_sockfd); //断开连接请求
void type_e(int client_sockfd); //未知类型请求

#endif