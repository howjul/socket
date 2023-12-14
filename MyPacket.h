#ifndef MYPACKET_H
#define MYPACKET_H

#include<iostream>
#include<string>
#include<optional>

#define PACKETFLAG "zhz&nzh" //消息包标志

class MyPacket
{
  private:
    char type; //消息类型
    char target; //发送目标
    std::string message; //发送内容
    
  public:
    MyPacket(); //构造函数
    MyPacket(char type);
    MyPacket(char type, std::string data);
    MyPacket(char type, std::string data, char target); //构造函数
    void init_packet(char type, std::string data = "1", char target = '1'); //初始化MyPacket
    char get_type(); //获取消息类型
    char get_target(); //获取消息目标
    std::string get_message(); //获取消息内容
    std::string to_string();  //将MyPacket转换为string
};

std::optional<MyPacket> to_MyPacket(std::string recv_mes); //将收到的string转换为MyPacket

#endif