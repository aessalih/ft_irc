#ifndef BOARD_HPP
#define BOARD_HPP

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "../mandatory/Server.hpp"

class Board
{
    private:
        char **content;
        int fd;
        int socket;
    public:
        static int flag;
        static int sockfd;
        Board();
        ~Board();
        void set_sock(int sock);
        int get_socket();
        std::vector<std::string> print_board();
        char get_char(int x, int y);
        int set_move(int x, int y, char player);
        char **getcontent();
        void set_fd(int file_d);
        int get_fd();
        std::string get_board();
};

void stopGame(int signum);
std::vector<std::string> split_bonus(const std::string& str);

#endif