#include"MyServer.h"
#include"MyPacket.h"

/************************************************服务器类主要函数*****************************************************/
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
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
  //表示监听0.0.0.0地址，socket只绑定端口，不绑定本主机的某个特定ip，让路由表决定传到哪个ip
  //server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  //绑定服务器套接字和服务器地址
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

MyServer::~MyServer(){ close(server_sockfd); }

void MyServer::on(){
  cout << "Server starts working!" << endl;
  cout << "Waiting for client to connect..." << endl;
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
    cout << endl << "A client connected!" << endl;
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

/************************************************服务器接口函数*****************************************************/
void MyServer::set_client_list(int list_num){ client_list_situation[list_num] = 1; }

void MyServer::rst_client_list(int list_num){
  client_list_situation[list_num] = 0;
  client_list.erase(list_num);
}

string MyServer::get_list(){
  stringstream ss;
  ss << endl;
  map<int, struct client_info>::iterator it;
  for (it = client_list.begin(); it != client_list.end(); it++) {
    ss << "id" << it->first + 1 << ": [";
    ss << inet_ntoa((it->second).client_addr.sin_addr) << " ";
    ss << ntohs((it->second).client_addr.sin_port);
    ss << "]" << endl;
  }
  return ss.str();
}

int MyServer::get_server_sockfd(){ return server_sockfd; }
void MyServer::set_is_end_true(){ _is_end = true; }
bool MyServer::is_end(){ return _is_end; }

/************************************************服务器辅助函数*****************************************************/
//获得一个客户端列表的序号
int MyServer::get_list_num(){
  for(int i = 0; i < MAX_CLIENT; i++)
    if(client_list_situation[i] == 0)
      return i;
  return -1;
}

//通过id查找客户端套接字
int MyServer::find_in_list(int id){
  if(client_list_situation[id] == 1) return client_list[id].client_sockfd;
  else return -1;
}

//检查客户端列表是否为空
bool MyServer::check_client_list(){
  for(int i = 0; i < MAX_CLIENT; i++){
    if(client_list_situation[i] == 1) return 1;
  }
  return 0;
}

/************************************************处理客户端请求函数*****************************************************/
//为客户端单独创建一个线程处理请求
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
    MyPacket recv_packet;
    auto recv = to_MyPacket(recv_str); 
    if(!recv.has_value()) continue;
    else recv_packet = recv.value();

    //处理请求
    cout << endl << "[" << list_num + 1 << "]handle request..." << endl;
    res = handle_request(recv_packet, info, list_num);

    //如果收到退出请求，则返回0
    if(res == 1){
      primary_server.set_is_end_true();
      break;
    } 
  }

  close(client_sockfd); //关闭客户端套接字
  std::lock_guard<std::mutex> lock(mutex);
  primary_server.rst_client_list(list_num); //释放

  //如果所有客户端都已经退出，则服务器退出
  if(primary_server.is_end() && !primary_server.check_client_list()){
    cout << "All the client disconnected! Server timinate!" << endl;
    close(primary_server.get_server_sockfd());
    exit(0);
  }
    
  return 0;
}

//处理单个请求
int handle_request(MyPacket request, struct thread_info info, int request_id){
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
    case 's': type_s(info.client_sockfd, request_id, targetid, message, *(info.server)); break;
    default: type_e(info.client_sockfd); break;
  }

  return 0;
}

/************************************************具体请求处理函数*****************************************************/
//成功连接响应
void type_h(int client_sockfd, sockaddr_in client_addr){
  //构造消息
  stringstream ss;
  ss << "hello! IP:" <<  inet_ntoa(client_addr.sin_addr) << " Port:" << ntohs(client_addr.sin_port);
  string message = ss.str();

  //发送消息
  MyPacket send_packet('h', message);
  cout << "   send messsage: " << message << endl;
  send(client_sockfd, send_packet.to_string().c_str(), send_packet.to_string().size(), 0);
}

//时间请求
void type_t(int client_sockfd){
  //构造消息
  time_t timep;
  struct tm *p;
  time(&timep); //获取从1970至今过了多少秒，存入time_t类型的timep
  p = localtime(&timep); //用localtime将秒数转化为struct tm结构体
  stringstream ss;
  ss << 1900 + p->tm_year << "年" << 1+ p->tm_mon << "月" << p->tm_mday << "日" << " ";
  ss << p->tm_hour << ":" << p->tm_min << ":" << p->tm_sec;

  //发送消息
  MyPacket send_packet('t', ss.str());
  cout << "   send messsage: " << ss.str() << endl;
  send(client_sockfd, send_packet.to_string().c_str(), send_packet.to_string().size(), 0);
}

//名字请求
void type_n(int client_sockfd){
  //构造消息
  char name[256];
  gethostname(name, sizeof(name));
  string hostname(name);

  //发送消息
  MyPacket send_packet('n', hostname);
  cout << "   send messsage: " << hostname << endl;
  send(client_sockfd, send_packet.to_string().c_str(), send_packet.to_string().size(), 0);
}

//列表读取请求
void type_l(int client_sockfd, MyServer server){
  //使用get_list读取并发送消息
  MyPacket send_packet('l', server.get_list());
  cout << "   send messsage: " << server.get_list();
  send(client_sockfd, send_packet.to_string().c_str(), send_packet.to_string().size(), 0);
}

//转发请求
void type_s(int client_sockfd, int client_id, char targetid, string message, MyServer server){
  //首先检查目标id是否存在
  int target_sockfd = server.find_in_list(targetid - 1);
  if(target_sockfd == -1){
    type_a(client_sockfd, "target id not exist!");
    return;
  }

  //构造消息
  string mes = "From [";
  mes.push_back(client_id + 1 + '0');
  mes.append("]: ");
  mes.append(message);

  //发送消息
  MyPacket send_packet('s', mes);
  cout << "   send messsage to ["<< (int)targetid  << "]: " << mes << endl;
  int status = send(target_sockfd, send_packet.to_string().c_str(), send_packet.to_string().size(), 0);

  //发送确认回复
  if(status < 0){
    type_a(client_sockfd, "Send failed!");
  }else{
    type_a(client_sockfd, "Already send the message!");
  }
}

//转发确认回复
void type_a(int client_sockfd, string message){
  //发送传入的messaeg
  MyPacket send_packet('a', message);
  cout << "   send messsage: " << message << endl;
  send(client_sockfd, send_packet.to_string().c_str(), send_packet.to_string().size(), 0);
}

//断开连接请求
void type_d(int client_sockfd){
  //构造消息
  string message("disconnect");

  //发送消息
  MyPacket send_packet('d', message);
  cout << "   send messsage: " << message << endl;
  send(client_sockfd, send_packet.to_string().c_str(), send_packet.to_string().size(), 0);
}

//未知类型请求
void type_e(int client_sockfd){
  //构造消息
  string message("error request type");

  //发送消息
  MyPacket send_packet('e', message);
  cout << "   send messsage: " << message << endl;
  send(client_sockfd, send_packet.to_string().c_str(), send_packet.to_string().size(), 0);
}


/************************************************主函数*****************************************************/
int main(){
  primary_server.on();
  return 0;
}