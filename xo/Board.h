#ifndef BOARD_H
#define BOARD_H

#include <iostream>
class Board
{
    private:
        char **content;
    public:
        Board();
        ~Board();
        void print_board();
        char get_char(int x, int y);
        int set_move(int x, int y, char player);
        char **getcontent();



};
#endif