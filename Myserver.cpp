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
  //bind(server_sockfd, (sockaddr*)&server_addr, sizeof(server_addr));
  if(bind(server_sockfd, (sockaddr*)&server_addr, sizeof(server_addr)) != 0){
    close(server_sockfd);
    cout << "Bind error!" << endl;
    exit(1);
  }
  cout << "Bind PORT" << PORT << " succeed!" << endl;
  
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
    cout << "Waiting for client to connect..." << endl;
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
    set_client_list(list_num);

    //创建线程处理客户端请求
    pthread_t thread;
    struct thread_info info = {list_num, client_sockfd, client_addr, this};
    pthread_create(&thread, NULL, handle_client, (void *)&info);
  }
}

int MyServer::get_list_num(){
  for(int i = 0; i < MAX_CLIENT; i++)
    if(client_list_situation[i] == 0)
      return i;
  return -1;
}

string MyServer::get_list(){
  stringstream ss;
  ss << endl;
  map<int, struct client_info>::iterator it;
  for (it = client_list.begin(); it != client_list.end(); it++) {
    ss << "id" << it->first << ": [";
    ss << inet_ntoa((it->second).client_addr.sin_addr);
    ss << ntohs((it->second).client_addr.sin_port);
    ss << "]" << endl;
  }
  return ss.str();
}

int MyServer::find_in_list(int id){
  if(client_list_situation[id] == 1) return client_list[id].client_sockfd;
  else return -1;
}

void MyServer::set_client_list(int list_num){
  client_list_situation[list_num] = 1;
}

void MyServer::rst_client_list(int list_num){
  client_list_situation[list_num] = 0;
}

void *handle_client(void* thread_info){
  //接受参数
  struct thread_info info = *((struct thread_info*)thread_info);
  int list_num = info.list_num;
  int client_sockfd = info.client_sockfd;
  sockaddr_in client_addr = info.client_addr;
  MyServer server = *(info.server);

  //发送成功连接响应
  type_h(client_sockfd, client_addr);

  char buffer[MAX_BUFFER];
  //循环进行消息处理
  while(true){
    //调用recv函数接收数据
    ssize_t res = recv(client_sockfd, buffer, MAX_BUFFER, 0);
    //若recv返回0，则连接已经关闭，直接break
    if(res == 0) break; 

    //将收到的数组转换为数据包
    string recv_str(buffer);
    MyPacket recv_packet = to_MyPacket(recv_str);

    //处理请求
    cout << "[" << list_num << "]handle request..." << endl;
    res = handle_request(recv_packet, info);

    //如果收到退出请求，则返回0
    if(res == 1) break;
  }

  close(client_sockfd); //关闭客户端套接字
  server.rst_client_list(list_num); //释放
  exit(0);
}

int handle_request(MyPacket request, struct thread_info info){
  //拆开数据包
  char type = request.get_type();
  char targetid = request.get_target();
  string message = request.get_message();

  switch (type)
  {
    case 't': type_t(info.client_sockfd); break;
    case 'n': type_n(info.client_sockfd); break;
    case 'd': type_d(info.client_sockfd); return 1;
    case 'l': type_l(info.client_sockfd, *(info.server)); break;
    case 's': type_s(info.client_sockfd, targetid, message, *(info.server)); break;
    default: type_e(info.client_sockfd); break;
  }

  return 0;
}

void type_t(int client_sockfd){
  time_t timep;
  struct tm *p;

  time(&timep); //获取从1970至今过了多少秒，存入time_t类型的timep
  p = localtime(&timep); //用localtime将秒数转化为struct tm结构体

  stringstream ss;
  ss << 1900 + p->tm_year << "年" << 1+ p->tm_mon << "月" << p->tm_mday << "日" << " ";
  ss << p->tm_hour << ":" << p->tm_min << ":" << p->tm_sec << endl;

  MyPacket send_packet('t', ss.str());
  cout << " send messsage: " << ss.str() << endl;
  send(client_sockfd, send_packet.to_string().c_str(), send_packet.to_string().size(), 0);
}

void type_n(int client_sockfd){
  char name[256];
  gethostname(name, sizeof(name));
  string hostname(name);

  MyPacket send_packet('n', hostname);
  cout << " send messsage: " << hostname << endl;
  send(client_sockfd, send_packet.to_string().c_str(), send_packet.to_string().size(), 0);
}

void type_d(int client_sockfd){
  string message("disconnect");

  MyPacket send_packet('d', message);
  cout << " send messsage: " << message << endl;
  send(client_sockfd, send_packet.to_string().c_str(), send_packet.to_string().size(), 0);
}

void type_e(int client_sockfd){
  string message("error request type");

  MyPacket send_packet('e', message);
  cout << " send messsage: " << message << endl;
  send(client_sockfd, send_packet.to_string().c_str(), send_packet.to_string().size(), 0);
}

void type_h(int client_sockfd, sockaddr_in client_addr){
  stringstream ss;
  ss << "hello! IP:" <<  inet_ntoa(client_addr.sin_addr) << " Port:" << ntohs(client_addr.sin_port) << endl;
  string message = ss.str();

  MyPacket send_packet('h', message);
  cout << " send messsage: " << message << endl;
  send(client_sockfd, send_packet.to_string().c_str(), send_packet.to_string().size(), 0);
}

void type_l(int client_sockfd, MyServer server){
  MyPacket send_packet('l', server.get_list());
  cout << " send messsage: " << endl << server.get_list() << endl;
  send(client_sockfd, send_packet.to_string().c_str(), send_packet.to_string().size(), 0);
}

void type_s(int client_sockfd, char targetid, string message, MyServer server){
  int target_sockfd = server.find_in_list(targetid);
  if(target_sockfd == -1){
    type_a(client_sockfd, "target id not exist!");
    return;
  }

  MyPacket send_packet('s', message);
  cout << " send messsage: " << message << endl;
  send(target_sockfd, send_packet.to_string().c_str(), send_packet.to_string().size(), 0);
  type_a(client_sockfd, "already send the message!");
}

void type_a(int client_sockfd, string message){
  MyPacket send_packet('a', message);
  cout << " send messsage: " << message << endl;
  send(client_sockfd, send_packet.to_string().c_str(), send_packet.to_string().size(), 0);
}


int main(){
  MyServer server;
  server.on();
  return 0;
}