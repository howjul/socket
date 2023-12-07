#include"MyServer.h"
#include"MyPacket.h"

MyServer::MyServer()
{ 
  //占用情况全部清零
  for(int i = 0; i < MAX_CLIENT; i++)
    client_list_situation[i] = 0;

  //初始化服务器套接字
  server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(server_sockfd == -1){
    cout << "Server socket error!" << endl;
    exit(1);
  }

  //初始化服务器地址
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //表示监听0.0.0.0地址，socket只绑定端口，不绑定本主机的某个特定ip，让路由表决定传到哪个ip

  //绑定服务器套接字和服务器地址
  bind(server_sockfd, (sockaddr*)&server_addr, sizeof(server_addr));
  // if(bind(server_sockfd, (sockaddr*)&server_addr, sizeof(server_addr)) != 0){
  //   close(server_sockfd);
  //   cout << "Bind error!" << endl;
  //   exit(1);
  // }
  
  //监听服务器套接字
  if(listen(server_sockfd, MAX_BACKLOG) != 0){
    close(server_sockfd);
    cout << "Listen error!" << endl;
    exit(1);
  }

  //输出相关消息
  cout << "Server successfully initialized!" << endl;
}

MyServer::~MyServer(){
  close(server_sockfd);
}

void MyServer::on(){
  cout << "Server starts working!" << endl;
  while(true){
    //接受客户端连接
    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_sockfd = accept(server_sockfd, (sockaddr*)&client_addr, &client_addr_len);
    if(client_sockfd == -1){
      cout << "Accept error!" << endl;
      continue;
    }

    //输出相关消息
    cout << "A client connected!" << endl;
    cout << "   Client IP: " << inet_ntoa(client_addr.sin_addr) << endl;
    cout << "   Client port: " << ntohs(client_addr.sin_port) << endl;

    //将客户端信息加入客户端列表
    int list_num = get_list_num();
    client_list[list_num].client_sockfd = client_sockfd;
    client_list[list_num].client_addr = client_addr;
    client_list_situation[list_num] = 1;

    //创建线程处理客户端请求
    pthread_t thread;
    struct thread_info info = {list_num, client_sockfd, client_addr};
    pthread_create(&thread, NULL, handle_client, (void *)&info);
  }
}

int MyServer::get_list_num(){
  for(int i = 0; i < MAX_CLIENT; i++)
    if(client_list_situation[i] == 0)
      return i;
  return -1;
}

void *handle_client(void* thread_info){
  //TODO
  exit(0);
}

int main(){
  MyServer server;
  server.on();
  return 0;
}