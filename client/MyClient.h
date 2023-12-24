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
#include <cstring>
#include <chrono>
#define head_signal "zhz&nzh"
#define MAXSIZE 1024
#define test 1
bool is_connected = false;
bool first_use = true;
int tcp_socket;
std::mutex mtx; 

class Validator {
public:
    static bool is_valid_op(const std::string& input);
    static bool is_valid_ip(const std::string& input);
    static bool is_valid_port(const std::string& input);
    static bool is_valid_id(const std::string& input);
};

class Interactor {
public:
    static std::optional<std::string> input_until_valid(bool (*check_func)(const std::string&)= [](const std::string&) { return true; });
    static int print_menu_list();
    static std::optional<std::pair<std::string, unsigned int>> input_ip_and_port();
    static std::optional<std::pair<char, std::string>> input_id_and_msg();
};