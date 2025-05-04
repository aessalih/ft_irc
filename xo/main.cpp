#include "Board.h"
#include <iostream>
#include <limits>

int char_digit(char c)
{
    if (c >= '0' && c <= '9')
        return (c - '0');
    else 
        return (-1);
}

int iswinningmove(char** board, char player, int row, int col) {
    board[row][col] = player;
    int win =
        (board[row][0] == player && board[row][1] == player && board[row][2] == player) ||
        (board[0][col] == player && board[1][col] == player && board[2][col] == player) ||
        (row == col && board[0][0] == player && board[1][1] == player && board[2][2] == player) ||
        (row + col == 2 && board[0][2] == player && board[1][1] == player && board[2][0] == player);
    board[row][col] = ' ';
    return win;
}

int getBestMove(char **content, char player, int& out_x, int& out_y) {
    char bot;

    if (player == 'X')
        bot = 'O';
    else
        bot = 'X';

        
    for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
    if (content[i][j] == ' ' && iswinningmove(content, bot, i, j)) {
        out_x = i;
        out_y = j;
        return (1);
    }

    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            if (content[i][j] == ' ' && iswinningmove(content, player, i, j)) {
                out_x = i;
                out_y = j;
                return (0);
            }

    if (content[1][1] == ' ') {
        out_x = 1;
        out_y = 1;
        return 0;
    }

    int corners[4][2] = { {0,0}, {0,2}, {2,0}, {2,2} };
    for (int i = 0; i < 4; ++i) {
        int x = corners[i][0];
        int y = corners[i][1];
        if (content[x][y] == ' ') {
            out_x = x;
            out_y = y;
            return 0;
        }
    }

    int sides[4][2] = { {0,1}, {1,0}, {1,2}, {2,1} };
    for (int i = 0; i < 4; ++i) {
        int x = sides[i][0];
        int y = sides[i][1];
        if (content[x][y] == ' ') {
            out_x = x;
            out_y = y;
            return 0;
        }
    }
    out_x = -1;
    return (0);
}


int main()
{
    Board board;

    std::string player;
    std::string move;
    char bot;
    int x_bot;
    int y_bot;


    int winner = 0;

    std::cout << "choose X or O:  ";
    std::getline(std::cin, player);
    if (player == "X")
        bot = 'O';
    else if (player == "O")
        bot = 'X';
    else
    {
        std::cout << "wrong char\n";
        exit(1);
    }
    board.print_board();
    char **content = board.getcontent();
    while (!winner)
    {
        std::cout << "choose where to put " << player << " : ";
        std::getline(std::cin, move);
        if (move.length() != 3)
        {
            std::cout << "wrong move\n";
            continue ;
        }
        int row = char_digit(move[0]);
        int col = char_digit(move[2]);
        if (row < 0 || row > 2 || col < 0 || col > 2) {
            std::cout << "invalid position\n";
            continue;
        }
        if (board.set_move(row, col, player[0]))
        {
            std::cout << "invalid position\n";
            continue ;
        }
        int check = getBestMove(content, player[0], x_bot, y_bot);
        if (x_bot == -1)
        {
            std::cout << "its a draw ------------\n";
            return 0;
        }
        if (board.set_move(x_bot, y_bot, bot))
        {
            std::cout << "invalid position\n";
            continue ;
        }
        board.print_board();
        if (check == 1)
        {
            std::cout << "i won ------------\n";
            return 0;
        }
    }

}
