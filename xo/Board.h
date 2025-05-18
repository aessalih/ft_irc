#ifndef BOARD_H
#define BOARD_H

#include <limits>
#include <iostream>
#include "../Server.hpp"
class Board
{
    private:
        char **content;
        int fd;
    public:
        Board();
        ~Board();
        void print_board();
        char get_char(int x, int y);
        int set_move(int x, int y, char player);
        char **getcontent();
        void set_fd(int file_d);
        int get_fd();



};
#endif