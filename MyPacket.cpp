#include"MyPacket.h"

MyPacket::MyPacket(){}

MyPacket::MyPacket(char type, std::string data = "", char target = 0)
{
  this->type = type;
  this->target = target;
  this->message = data;
}

void MyPacket::init_packet(char type, std::string data = "", char target = 0)
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
  res.init_packet(recv_mes[7], recv_mes.substr(9), recv_mes[8]);
  return res;
}
