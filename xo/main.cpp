#include "Board.h"

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


int play(int fd)
{
    Board board;

    std::string player;
    std::string move;
    char bot;
    int x_bot;
    int y_bot;

    board.set_fd(fd);
    int winner = 0;

    send(fd, "choose X or O: ", 15, 0);
    char buffer[1024];
    memset(buffer, 0, 1024);
    int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes > 0) {
        buffer[bytes] = '\0'; 
        std::string player(buffer);
        std::cout << "|" << player << "|" << "\n";
    }

    if (player == "X\n")
        bot = 'O';
    else if (player == "O\n")
        bot = 'X';
    else
    {
        send(fd, "wrong char\n", 11, 0);
        return 0;
    }
    board.print_board();
    char **content = board.getcontent();
    while (!winner)
    {
        send(fd, "choose where to put ", 20, 0);
        send(fd, player.c_str(), player.length(), 0);
        send(fd, " : ", 3, 0);
        memset(buffer, 0, 1024);
        bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);

        if (bytes > 0) {
            buffer[bytes] = '\0'; 
            std::string move(buffer);
        }
        if (move.length() != 4)
        {
            send(fd, "wrong move\n", 11, 0);
            continue ;
        }
        int row = char_digit(move[0]);
        int col = char_digit(move[2]);
        if (row < 0 || row > 2 || col < 0 || col > 2) {
            send(fd, "invalid position\n", 17, 0);
            continue;
        }
        if (board.set_move(row, col, player[0]))
        {
            send(fd, "invalid position\n", 17, 0);
            continue ;
        }
        int check = getBestMove(content, player[0], x_bot, y_bot);
        if (x_bot == -1)
        {
            send(fd, "its a draw ------------\n", 24, 0);
            return 0;
        }
        if (board.set_move(x_bot, y_bot, bot))
        {
            send(fd, "invalid position\n", 17, 0);
            continue ;
        }
        board.print_board();
        if (check == 1)
        {
            send(fd, "i won ------------\n", 19, 0);
            return 0;
        }
    }
    return 0;
}
