
#include <iostream>
#include "MyPacket.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <map>
#include <limits>
#include <thread>
#include <mutex>
#include <regex>
#include <string>
#include <optional>
#include <chrono>
#define head_signal "zhz&nzh"
#define MAXSIZE 1024
bool is_connected = false;
bool first_use = true;
int tcp_socket;


std::mutex mtx; 

bool is_valid_op(const std::string& input) {
    if (!std::isdigit(input[0]))
        throw std::runtime_error("非法输入!");

    int op = std::stoi(input);
    return ((is_connected && op <= 6 && op >= 1) || (!is_connected && op <= 2 && op >= 1));
}

bool is_valid_ip(const std::string& input) {
    // IP 地址的正则表达式模式
    std::regex pattern(R"((\b(?:\d{1,3}\.){3}\d{1,3}\b))");

    // 匹配输入是否符合 IP 地址的模式
    return std::regex_match(input, pattern);
} 

bool is_valid_port(const std::string& input) {
    int port = std::stoi(input);
    return (port >= 0 && port <= 65535);
}

bool is_valid_id(const std::string& input) {
    int id = std::stoi(input);
    return (id >= 0 && id <= 255);
}

void recv_msg_thread(){
    std::string buf_str;
    // std::this_thread::sleep_for(std::chrono::seconds(5));
    while(is_connected){
        char buf[MAXSIZE];
        memset(buf, 0, sizeof(buf));
        recv(tcp_socket, buf, sizeof(buf) - 1, 0);
        auto p = to_MyPacket(buf);
        memset(buf, 0, sizeof(buf));
        MyPacket recv_packet;
        if(!p.has_value()) continue;
        else recv_packet = p.value();
        std::cout << "\033[35m[Server]\033[0m " << recv_packet.message << std::endl;
    }
}

std::optional<std::string> input_until_valid(bool (*check_func)(const std::string&)= [](const std::string&) { return true; }){
    std::string str;
    while(true){
        try {
            std::cin >> str;
            if (str == "-1") 
                return std::nullopt;
            
            else if(check_func(str))
                return str;

            else
                throw std::runtime_error("非法输入!");
        } catch (const std::exception& e) {
            std::cout << "\033[31m[System]\033[0m 发生异常: " << e.what() << std::endl;
            std::cout << "\033[32m[System]\033[0m 请重新输入: " << std::endl;
            std::cout << "\033[34m[Client]\033[0m ";
            std::cin.clear(); // 清除输入流的错误状态
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 忽略剩余的输入内容
        }        
    }    
}

int print_menu_list(){
    std::cout << "\033[33m************功能菜单************" << std::endl;
    if(!is_connected){
        std::cout << "* 1. 连接                      *" << std::endl;
        std::cout << "* 2. 退出                      *" << std::endl;
    }
    else{
        std::cout << "* 1. 获取时间                  *" << std::endl;
        std::cout << "* 2. 获取名字                  *" << std::endl;
        std::cout << "* 3. 获取客户端列表            *" << std::endl;
        std::cout << "* 4. 发送消息                  *" << std::endl;
        std::cout << "* 5. 断开连接                  *" << std::endl;
        std::cout << "* 6. 退出                      *" << std::endl;
    }
    std::cout << "********************************\033[0m" << std::endl;
    std::cout << "\033[32m[System]\033[0m 请输入序号选择功能:" << std::endl;
    std::cout << "\033[34m[Client]\033[0m ";

    
    std::optional<std::string> op = input_until_valid(is_valid_op);
    if(op.has_value()) 
        return std::stoi(op.value());
    else{
        if(!is_connected) 
            return 2; // 未连接状态下的退出
        else 
            return 6; // 连接状态下的退出
    }
}

std::optional<std::pair<std::string, unsigned int>> input_ip_and_port(){
    std::optional<std::string> ip;
    std::optional<std::string> port;
    std::pair<std::string, unsigned int> dst;

    std::cout << "\033[32m[System]\033[0m 请输入服务器的IP地址:(-1强制退出)" << std::endl;
    ip = input_until_valid(is_valid_ip);
    if(ip.has_value()) 
        dst.first = ip.value();
    else
        return std::nullopt;
    
    std::cout << "\033[32m[System]\033[0m 请输入服务器的端口:(-1强制退出)" << std::endl;
    port = input_until_valid(is_valid_port);
    if(port.has_value()) 
        dst.second = std::stoul(port.value());
    else
        return std::nullopt;

    return dst;
}

std::optional<std::pair<char, std::string>> input_id_and_msg(){
    std::optional<std::string> id;
    std::optional<std::string> msg;
    std::pair<char, std::string> dst;

    std::cout << "\033[32m[System]\033[0m 请输入发送客户端的列表编号:(-1强制退出)" << std::endl;
    id = input_until_valid(is_valid_id);
    if(id.has_value()) 
        dst.first = std::stoi(id.value());
    else
        return std::nullopt;
    
    std::cout << "\033[32m[System]\033[0m 请输入发送内容:(-1强制退出)" << std::endl;
    msg = input_until_valid();
    if(msg.has_value()) 
        dst.second = msg.value();
    else
        return std::nullopt;

    return dst;
}

int main(){
    while(true){
        if(first_use) {
            first_use = false;
            std::cout << "\033[32m[System]\033[0m 欢迎!" << std::endl;
        }
        int op = print_menu_list();

        /* 退出程序 */
        if(op == 2 && !is_connected){
            break;
        } 

        /* 连接服务端 */
        if(op == 1 && !is_connected){
            tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
            auto dst = input_ip_and_port();
            if(!dst.has_value()) break;

            struct sockaddr_in serv_addr;
            memset(&serv_addr, 0, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(dst.value().second); //端口号需要转为网络序
            serv_addr.sin_addr.s_addr = inet_addr(dst.value().first.c_str());

            if(connect(tcp_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != -1){
                std::cout << "\033[32m[System]\033[0m 连接成功!" << std::endl;
                is_connected = true;
                std::thread t(recv_msg_thread);
                t.detach();
            }
            else{
                std::cout << "\033[31m[System]\033[0m 连接失败!" << std::endl;
                is_connected = false;
            }
            continue;
        }

        /* 向服务端发送服务请求 */
        if(is_connected){
            MyPacket msg_s;
            std::optional<std::pair<char, std::string>> dst;
            switch(op){
                case 1:
                    msg_s.init_packet('t');
                    break;

                case 2:
                    msg_s.init_packet('n');
                    break;

                case 3:
                    msg_s.init_packet('l');
                    break;
                
                case 4: 
                    dst = input_id_and_msg();
                    if(dst.has_value())
                        msg_s.init_packet('s', dst.value().second, dst.value().first);
                    else
                        throw("无效id, 请查询正确的客户端id后再发送!");
                    break;
                
                case 5:
                    msg_s.init_packet('d');
                    is_connected = false;
                    break;

                case 6:
                    msg_s.init_packet('d');
                    is_connected = false;
                    break;

                default:
                    break;

            }
            try{
                std::string str = msg_s.to_string();
                const char* msg = str.c_str();
                if(send(tcp_socket, msg, str.size(), 0) == -1)
                    throw("请求发送失败!");
                else{
                    std::cout << "\033[32m[System]\033[0m 请求成功发送, 请稍等..." << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
                if(!is_connected)
                    close(tcp_socket);
            } catch (const std::exception& e) {
                std::cout << "\033[31m[System]\033[0m 发生异常: " << e.what() << std::endl;
            }        
        }
    }
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "\033[32m[System]\033[0m 再见!" << std::endl;
    return 0;
}