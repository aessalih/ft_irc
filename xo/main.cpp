#include "Board.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

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

int playerWon(char **content, char player)
{
    for (int i = 0; i < 3; ++i)
        if (content[i][0] == player && content[i][1] == player && content[i][2] == player)
            return 1;

    for (int j = 0; j < 3; ++j)
        if (content[0][j] == player && content[1][j] == player && content[2][j] == player)
            return 1;

    if (content[0][0] == player && content[1][1] == player && content[2][2] == player)
        return 1;

    if (content[0][2] == player && content[1][1] == player && content[2][0] == player)
        return 1;

    return 0;
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

std::string parse_response(std::string input)
{
    size_t last_colon = input.find_last_of(':');
    if (last_colon != std::string::npos) {
        return input.substr(last_colon + 1);
    }
    return input;
}

std::vector<std::string> split(const std::string& str) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, ' ')) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}
int play(int fd, std::string current_player);
std::string parse_sender(const std::string& message) {
    if (message.empty() || message[0] != ':')
        return "";
    
    size_t space_pos = message.find(' ');
    if (space_pos == std::string::npos)
        return "";
    
    return message.substr(1, space_pos - 1);
}

bool handle_message(int sockfd, const std::string& message) {
    std::string sender = parse_sender(message);
    if (sender.empty())
        return false;
    std::cout << "Sender: " << sender << std::endl;

    if (message.find("PRIVMSG") != std::string::npos) {
        std::string content = parse_response(message);
        content.erase(content.find_last_not_of(" \n\r\t") + 1);
        content.erase(0, content.find_first_not_of(" \n\r\t"));
                
        if (content == "play") {
            std::string response = "PRIVMSG " + sender + " :Starting a new game of Tic Tac Toe!\n";
            send(sockfd, response.c_str(), response.length(), 0);
            sleep(1);
            play(sockfd, sender);
            return true;
        } else {
            std::string response = "PRIVMSG " + sender + " :To play Tic Tac Toe, send 'play'\n";
            send(sockfd, response.c_str(), response.length(), 0);
        }
    }
    return false;
}

int play(int fd, std::string current_player) {
    Board board;
    std::string player;
    std::string move;
    char bot;
    int x_bot;
    int y_bot;

    board.set_fd(fd);
    int winner = 0;

    char buffer[1024];

    std::string response = "PRIVMSG " + current_player + " Choose X or O: ";
    send(fd, response.c_str(), response.length(), 0);

    while (true) {
        memset(buffer, 0, 1024);
        recv(fd, buffer, sizeof(buffer) - 1, 0);
        player = buffer;
        player = parse_response(player);
        player.erase(player.find_last_not_of(" \n\r\t") + 1);
        player.erase(0, player.find_first_not_of(" \n\r\t"));
        if (player == "X" || player == "O")
        {
            break;
        }
        response = "PRIVMSG " + current_player + " Invalid choice. Please choose X or O: ";
        send(fd, response.c_str(), response.length(), 0);
    }

    if (player == "X")
        bot = 'O';
    else
        bot = 'X';
    std::vector<std::string> board_str = board.print_board();
    for(size_t i = 0; i < board_str.size(); i++)
    {
    response = "PRIVMSG " + current_player + " " + board_str[i] + "\n";
    std::cout << board_str[i] + "/n";
    send(fd, response.c_str(), response.length(), 0);
    sleep(1);
    }
    char **content = board.getcontent();
    while (!winner) {
        response = "PRIVMSG " + current_player + " choose where to put " + player + " (like this  row[separator]col): ";
        send(fd, response.c_str(), response.length(), 0);

        while (true) {
            memset(buffer, 0, 1024);
            recv(fd, buffer, sizeof(buffer) - 1, 0);
            move = buffer;
            move.erase(move.find_last_not_of(" \n\r\t") + 1);
            move.erase(0, move.find_first_not_of(" \n\r\t"));
            move = parse_response(move);
            if (move.length() != 3)
            {
            response = "PRIVMSG " + current_player + " wrong move format. Use row,col (like this 0,0)\n";
            send(fd, response.c_str(), response.length(), 0);
            continue;
            }
            int row = char_digit(move[0]);
            int col = char_digit(move[2]);
            if (row < 0 || row > 2 || col < 0 || col > 2) {
                response = "PRIVMSG " + current_player + " invalid position.use numbers 0-2\n";
                send(fd, response.c_str(), response.length(), 0);
                continue;
            }

            if (board.set_move(row, col, player[0])) {
                response = "PRIVMSG " + current_player + " position alreadytaken\n";
                send(fd, response.c_str(), response.length(), 0);
                continue;
            }
            else 
                break ;
        }

    
         if (playerWon(content, player[0]))
         {
                response = "PRIVMSG " + current_player + " You won! ------------\n";
                send(fd, response.c_str(), response.length(), 0);
                return 0;
        }

        int check = getBestMove(content, player[0], x_bot, y_bot);
        if (x_bot == -1) {
            response = "PRIVMSG " + current_player + " It s a draw! ------------\n";
            send(fd, response.c_str(), response.length(), 0);
            return 0;
        }

        if (board.set_move(x_bot, y_bot, bot)) {
            response = "PRIVMSG " + current_player + " Error in bot move\n";
            send(fd, response.c_str(), response.length(), 0);
            continue;
        }
        
        board_str = board.print_board();
        for(size_t i = 0; i < board_str.size(); i++)
        {
        response = "PRIVMSG " + current_player + " " + board_str[i] + "\n";
        std::cout << board_str[i] + "/n";
        send(fd, response.c_str(), response.length(), 0);
        sleep(1);
        }
        if (check == 1) {
            response = "PRIVMSG " + current_player + " I won! ------------\n";
            send(fd, response.c_str(), response.length(), 0);
            return 0;
        }
    }
    return 0;
}


int main(int ac, char **av) {
    if (ac != 4) {
        std::cout << "Usage: " << av[0] << " <port> <nickname> <password>\n";
        return 1;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error creating socket\n";
        return 1;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(std::atoi(av[1]));
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.0");

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection failed\n";
        return 1;
    }
    char buffer[1024];
    int bytes = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) {
        std::cerr << "Connection closed or error\n";
        return 1;
    }

    std::string pass_cmd = "pass " + std::string(av[3]) + "\n";
    std::string nick_cmd = "nick " + std::string(av[2]) + "\n";
    std::string user_cmd = "user " + std::string(av[2]) + "\n";

    send(sockfd, pass_cmd.c_str(), pass_cmd.length(), 0);
    sleep(1);
    
    send(sockfd, nick_cmd.c_str(), nick_cmd.length(), 0);
    sleep(1);
    
    send(sockfd, user_cmd.c_str(), user_cmd.length(), 0);
    sleep(1);

    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        bytes = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            std::cerr << "Connection closed or error\n";
            break;
        }

        std::string message(buffer);
        handle_message(sockfd, message);
    }

    close(sockfd);
    return 0;
}
