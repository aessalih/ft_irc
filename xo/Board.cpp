#include "Board.h"

Board::Board()
{
    content = new char*[3];
    for (int i = 0; i < 3; ++i) {
        content[i] = new char[3];
        for (int j = 0; j < 3; ++j)
            content[i][j] = ' ';
    }
}

Board::~Board()
{

}

void Board::print_board()
{
    int i = 0;
    int j = 0;
    while (i < 7)
    {
        j =  0;
        while (j < 7)
        {
            if (i % 2 != 0 && j % 2 == 0)
                send(fd, "|", 1, 0);
            else if (i % 2 == 0 && j % 2 != 0)
                send(fd, "-", 1, 0);
            else if  (i % 2 != 0 && j % 2 != 0)
                send(fd, (void *)&(char const&)(get_char(i / 2, j / 2)), 1, 0);
            else
                send(fd, " ", 1, 0);
            j++;
        }     
        send(fd, "\n", 1, 0);
        i++;
    }
}

char Board::get_char(int x, int y)
{
    return (content[x][y]);
}

int Board::set_move(int x, int y, char player)
{
    if (content[x][y] == ' ')
        content[x][y] = player;
    else
        return (1);
    return (0);        
}

char **Board::getcontent()
{
    return (this->content);
}

void Board::set_fd(int file_d)
{
    fd = file_d;
}

int Board::get_fd()
{
    return (fd);
}

