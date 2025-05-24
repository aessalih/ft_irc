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
    delete[] content;
}

std::vector<std::string> Board::print_board() {
    std::vector<std::string> lines;

    for (int i = 0; i < 7; ++i) {
        std::string line;
        for (int j = 0; j < 7; ++j) {
            if (i % 2 != 0 && j % 2 == 0)
                line += "|";
            else if (i % 2 == 0 && j % 2 != 0)
                line += "-";
            else if (i % 2 != 0 && j % 2 != 0)
                line += get_char(i / 2, j / 2);
            else
                line += "=";
        }
        lines.push_back(line);
    }

    return lines;
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

std::string Board::get_board() 
{
    std::string result;
   
    return result;
}

