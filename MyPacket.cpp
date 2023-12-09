#include"MyPacket.h"

MyPacket::MyPacket(){}

MyPacket::MyPacket(char type)
{
  this->type = type;
  this->target = 1;
  this->message = "";
}

MyPacket::MyPacket(char type, std::string data)
{
  this->type = type;
  this->target = 1;
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
  res += type;
  res += target;
  res += message;
  return res;
}

//将收到的string转换为MyPacket
MyPacket to_MyPacket(std::string recv_mes){
  MyPacket res;
  std::string flag = PACKETFLAG;
  res.init_packet(recv_mes[flag.length()], recv_mes.substr(flag.length() + 2), recv_mes[flag.length() + 1]);
  return res;
}
