#include"MyPacket.h"

MyPacket::MyPacket(){}

MyPacket::MyPacket(char type)
{
  this->type = type;
  this->target = '1';
  this->message = "";
}

MyPacket::MyPacket(char type, std::string data)
{
  this->type = type;
  this->target = '1';
  this->message = data;
}

MyPacket::MyPacket(char type, std::string data, char target)
{
  this->type = type;
  this->target = target;
  this->message = data;
}

void MyPacket::init_packet(char type, std::string data, char target)
{
  this->type = type;
  this->target = target;
  this->message = data;
}

char MyPacket::get_type() { return type; }

char MyPacket::get_target() { 
  //检查，只有消息类型为's'才能获得有效发送目标
  if(type == 's') return target;
  else return -1; 
}

std::string MyPacket::get_message() { return message; }

std::string MyPacket::to_string()
{
  std::string res = PACKETFLAG;
  res.push_back(type);
  res.push_back(target);
  res.append(message);
  return res;
}

//将收到的string转换为MyPacket
std::optional<MyPacket> to_MyPacket(std::string recv_mes){
  MyPacket res;
  std::string flag = PACKETFLAG;
  std::string message;

  //首先进行消息的检验
  if(recv_mes.length() <= 9 || recv_mes.substr(0, 7) != PACKETFLAG)
    return std::nullopt;

  //然后进行消息的解析
  try{
    message = recv_mes.substr(flag.length() + 2);
  }catch(std::exception& e){
    std::cerr << "Caught exception of type: " << typeid(e).name() << std::endl;
    std::cout << "Standard exception: " << e.what() << std::endl;  
    exit(1);
  }
  res.init_packet(recv_mes[flag.length()], message, recv_mes[flag.length() + 1]);
  
  return res;
}
