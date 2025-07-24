#ifndef BOARD_H
#define BOARD_H

#include <iostream>
#include "../mandatory/Server.hpp"
class Board
{
    private:
        char **content;
        int fd;
        int socket;
    public:
        Board();
        ~Board();
        void set_sock(int sock);
        std::vector<std::string> print_board();
        char get_char(int x, int y);
        int set_move(int x, int y, char player);
        char **getcontent();
        void set_fd(int file_d);
        int get_fd();
        std::string get_board();



};
#endif