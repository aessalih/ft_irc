#include "Server.hpp"
#include <sstream>

// canonical form and parameterize constructor definition
Server::Server() {
	this->opt = 1;
	this->addrlen = sizeof(address);
	memset(buffer, 0, 1024);
	this->port = 8080;
	if (this->port == -1)
		exit (EXIT_FAILURE);
	this->password = "1234";
}

Server::Server(char **av) {
	this->opt = 1;
	this->addrlen = sizeof(address);
	memset(buffer, 0, 1024);
	this->port = parse_port(av[1]);
	if (this->port == -1)
		exit (EXIT_FAILURE);
	this->password = av[2];
}

Server::Server(const Server& obj) {
	(void) obj;
}

Server&	Server::operator=(const Server& obj) {
	(void) obj;
	return (*this);
}

Server::~Server() {
	close(sockfd);
	close(accept_fd);
}

// methods definition
int	Server::init() {
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		std::cerr << ERROR << std::endl;
		std::cerr << "\033[1;31mSOCKET FUNCTION FAILS\033[0m\n";
		return -1;
	}
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		std::cerr << ERROR << std::endl;
		std::cerr << "\033[1;31mSET SOCKET OPRION FAILS\033[0m\n";
		return -1;
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(this->port);
	if (bind(sockfd, (struct sockaddr*)&address, addrlen) == -1) {
		std::cerr << ERROR << std::endl;
		std::cerr << "\033[1;31mBIND FUNCTION FAILS\033[0m\n";
		return -1;
	}
	if (listen(sockfd, 3) == -1) {
		std::cerr << ERROR << std::endl;
		std::cerr << "\033[1;31mLISTEN FAILS\033[0m\n";
		return -1;
	}
	server_sockfd.fd = sockfd;
	server_sockfd.events = POLLIN;
	server_sockfd.revents = 0;
	fds.push_back(server_sockfd);
	std::cout << LAUNCHED << "\033[1;32mON PORT: " << this->port << "\033[0m" << std::endl;
	return 0;
}

int Server::run() {
	while (true) {
		int	poll_count = poll(fds.data(), fds.size(), -1);
		if (poll_count < 0) {
			std::cerr << "\033[1;31mPOLL FUNCTION FAILS\033[0m\n";
			return -1;
		}
		for (size_t i = 0; i < fds.size(); i++) {
			if (fds[i].revents & POLLIN) {
				if (fds[i].fd == sockfd)
					handleNewClients();
				else
					handleClientMessage(i);
			}
			else if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
				std::cout << "Client disconnected (fd: " << fds[i].fd << ")\n";
				close(fds[i].fd);
				fds.erase(fds.begin() + i);
				clients.erase(clients.begin() + i - 1);
				i--;
			}
		}
	}
	return 0;
}

int	Server::handleNewClients()
{
	struct pollfd	newFd;
	struct sockaddr_in	clientAddr;
	int	addrlen = sizeof(clientAddr);

	accept_fd = accept(sockfd, (struct sockaddr*)&clientAddr, (socklen_t *)&addrlen);
	if (accept_fd < 0) {
		std::cerr << "\033[1;31mERROR ACCEPTING CONNECTION\033[0m\n";
		return -1;
	}
	newFd.fd = accept_fd;
	newFd.events = POLLIN;
	newFd.revents = 0;
	fds.push_back(newFd);
	Client	client(newFd.fd);
	clients.push_back(client);
	return (1);
}

void Server::handleClientMessage(size_t i) {
	memset(buffer, 0, 1024);

	int client_fd = fds[i].fd;
	int bytes = recv(client_fd, buffer, 1024, 0);
	if (bytes <= 0) {
		std::cout << "Client disconnected (fd : " << client_fd << ")\n";
		close(client_fd);
		fds.erase(fds.begin() + i);
		clients.erase(clients.begin() + i - 1);
		return ;
	}
	// check if client is registered
	std::cout << buffer;
	if (clients[i - 1].getIsRegistered() == false) {
		if (clients[i - 1].getHavePass() == false) {
			if (check_password(buffer, client_fd)) {
				clients[i - 1].setHavePass(true);
				return ;
			}
			return ;
		}
		if (!check_names(clients, i - 1, buffer, client_fd))
			return ;
		clients[i - 1].setIsRegestered(true);
		std::string msg = ":irc 001 " + clients[i - 1].getNickname() + " :Welcome to IRC\r\n";
		send(client_fd, msg.c_str(), msg.size(), 0);
		return ;
	}
	for (int i = 0; i < clients.size(); i++) {
		std::cout << "client" << i << " = " << clients[i].getNickname() << std::endl;
	}

	// parse the message
	std::string msg(buffer);
	std::vector<std::string> tokens = split(msg);
	if (tokens.empty())
		return;
	std::string cmd = tokens[0];
	if (cmd == "JOIN") {
        handleJoin(i, client_fd, tokens);
    }
    else if (cmd == "PRIVMSG") {
        handlePrivmsg(i, client_fd, tokens);
    }
    else if (cmd == "KICK") {
        handleKick(i, client_fd, tokens);
    }
    else if (cmd == "INVITE") {
        handleInvite(i, client_fd, tokens);
    }
    else if (cmd == "TOPIC") {
        handleTopic(i, client_fd, tokens);
    }
    else if (cmd == "MODE") {
        handleMode(i, client_fd, tokens);
    }
    else if (!cmd.empty()){
        std::string error_msg = ": 421 " + clients[i - 1].getNickname() + " " + cmd + " :Unknown command\r\n";
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
    }
}
// :PASS aksjdlq weiooiuda
int	Server::check_password(char *buffer, int fd) {
	char *pass = strtok(buffer, " ");
	std::string	password = pass;
	std::string response;
	
	password.erase(password.find_last_not_of("\r\n") + 1);
	if (password != "PASS")
	return 0;
	pass = strtok(NULL, " ");
	if (!pass) {
		response = ":irc 461 pass :need more params\r\n";
		send(fd, response.c_str() , response.size(), 0);
		return 0;
	}
	password = pass;
	password.erase(password.find_last_not_of("\r\n") + 1);
	if (password.empty()) {
		response = ":irc 461 pass :need more params\r\n";
		send(fd, response.c_str() , response.size(), 0);
		return 0;
	}
	if (password == this->password)
		return 1;
	response = ":irc 461 pass :need more params\r\n";
	send(fd, response.c_str() , response.size(), 0);
	return 0;
}

int	Server::check_names(std::vector<Client> &clients, size_t i, char *buffer, int fd) {
	std::string newBuffer = buffer;
	char *token = strtok(buffer, " ");
	size_t	j = 0;
	std::string response;

	std::string name = token;
	name.erase(name.find_last_not_of("\r\n") + 1);
	if (name == "NICK") {
		token = strtok(NULL, " ");
		if (!token) {
			response = ":irc 431 nick :no nickname given\r\n";
			send(fd, response.c_str(), response.size(), 0);
			return 0;
		}
		name = token;
		name.erase(name.find_last_not_of("\r\n") + 1);
		while (j < clients.size()) {
			if (clients[j].getNickname() == name) {
				response = ":irc 433 nick :nickname is already in use\r\n";
				send(fd, response.c_str(), response.size(), 0);
				return 0;
			}
			j++;
		}
		if (name.find_first_of(" #,*?!@") != std::string::npos) {
			response = ":irc 432 * "+ name +" : :Erroneous nickname\r\n";
			send(fd, response.c_str(), response.size(), 0);
			////////////////////////////////////////////////////////////
			return 0;
		}
		clients[i].setNickname(name);
		if (!clients[i].getUsername().empty())
			return 1;
		return 0;
	}
	else if (name == "USER") {
		if (split1(newBuffer).size() != 4) {
			response = ":irc 461 user : invalid username\r\n";
			send(fd, response.c_str(), response.size(), 0);
			return 0;
		}
		token = strtok(NULL, " ");
		if (!token) {
			response = ":irc 461 user : invalid username\r\n";
			send(fd, response.c_str(), response.size(), 0);
			return 0;
		}
		name = token;
		name.erase(name.find_last_not_of("\n") + 1);
		clients[i].setUsername(name);
		if (!clients[i].getNickname().empty())
			return 1;
		return 0;
	}
	if (!clients[i].getNickname().empty() && !clients[i].getUsername().empty())
		return (1);
	return (0);
}

int	Server::parse_port(char *av) {
	int	i;

	for (i = 0; av[i]; i++) {
		if (av[i] < '0' || av[i] > '9')
			return -1;
	}
	return (atoi(av));
}

std::vector<std::string> split1(const std::string &s){
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	bool inWord = false;
	std::string currentWord;
	int flag = 0;

	for (size_t i = 0; i < s.length(); i++) {
		if (flag == 0 && (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || '\r')) {
			tokens.push_back(currentWord);
			currentWord.clear();
			inWord = false;
		} else {
			if (!inWord && s[i] == ':')
				flag = 1;
			currentWord += s[i];
			inWord = true;
		}
	}

	if (!currentWord.empty()) {
		tokens.push_back(currentWord);
	}

	return tokens;
}


// std::vector<std::string> split(const std::string &s) {
// 	std::vector<std::string> tokens;
// 	std::string currentWord;
// 	bool inWord = false;

// 	for (size_t i = 0; i < s.length(); i++) {
// 		if (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r') {
// 			if (inWord) {
// 				tokens.push_back(currentWord);
// 				currentWord.clear();
// 				inWord = false;
// 			}
// 			// If we encounter a newline, add an empty token if we're not in a word
// 			if ((s[i] == '\n' || s[i] == '\r') && !inWord) {
// 				tokens.push_back("");
// 			}
// 		} else {
// 			currentWord += s[i];
// 			inWord = true;
// 		}
// 	}

// 	if (inWord && !currentWord.empty()) {
// 		tokens.push_back(currentWord);
// 	}

// 	return tokens;
// }
std::vector<std::string> split(const std::string &s){
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	bool inWord = false;
	std::string currentWord;
	int flag = 0;

	for (size_t i = 0; i < s.length(); i++) {
		if (flag == 0 && (s[i] == ' ' || s[i] == '\t' || s[i] == '\n')) {
			tokens.push_back(currentWord);
			currentWord.clear();
			inWord = false;
		} else {
			if (!inWord && s[i] == ':')
				flag = 1;
			currentWord += s[i];
			inWord = true;
		}
	}

	if (!currentWord.empty()) {
		tokens.push_back(currentWord);
	}

	return tokens;
}

// void Server::handleMode(size_t i, int client_fd, const std::vector<std::string>& tokens) {
//     Client& client = clients[i - 1];
//     if (tokens.size() < 2) {
//         std::string response = ": 461 ";
//         response += client.getNickname();
//         response += " MODE :Not enough parameters\r\n";
//         send(client_fd, response.c_str(), response.size(), 0);
//         return;
//     }

//     std::string channelName = tokens[1];
//     if (channelName[0] != '#' && channelName[0] != '&') {
//         std::string response = ": 403 ";
//         response += client.getNickname();
//         response += " ";
//         response += channelName;
//         response += " :No such channel\r\n";
//         send(client_fd, response.c_str(), response.size(), 0);
//         return;
//     }

//     Channel* channel = getChannel(channelName);
//     if (!channel) {
//         std::string response = ": 403 ";
//         response += client.getNickname();
//         response += " ";
//         response += channelName;
//         response += " :No such channel\r\n";
//         send(client_fd, response.c_str(), response.size(), 0);
//         return;
//     }

//     // Check if user is in channel
//     bool user_in_channel = false;
//     const std::vector<Client>& channel_clients = channel->get_clients();
//     for (size_t j = 0; j < channel_clients.size(); j++) {
//         if (channel_clients[j] == client) {
//             user_in_channel = true;
//             break;
//         }
//     }

//     if (!user_in_channel) {
//         std::string response = ": 442 ";
//         response += client.getNickname();
//         response += " ";
//         response += channelName;
//         response += " :You're not on that channel\r\n";
//         send(client_fd, response.c_str(), response.size(), 0);
//         return;
//     }

//     // If no mode string provided, show current modes
//     if (tokens.size() == 2) {
//         std::string response = ": 324 ";
//         response += client.getNickname();
//         response += " ";
//         response += channelName;
//         response += " ";
//         response += channel->get_mode();
        
//         // Add mode arguments
//         std::string args;
//         if (channel->get_mode().find('k') != std::string::npos) {
//             std::stringstream ss;
//             ss << channel->get_key();
//             args += ss.str();
//             args += " ";
//         }
//         if (channel->get_mode().find('l') != std::string::npos) {
//             std::stringstream ss;
//             ss << channel->get_max_clients();
//             args += ss.str();
//             args += " ";
//         }
//         if (!args.empty()) {
//             response += " ";
//             response += args;
//         }
        
//         // Add all operators including creator
//         for (size_t j = 0; j < channel_clients.size(); j++) {
//             if (channel->isOperator(channel_clients[j]) || channel->getCreator() == channel_clients[j]) {
//                 response += " @";
//                 response += channel_clients[j].getNickname();
//             }
//         }
//         response += "\r\n";
//         send(client_fd, response.c_str(), response.size(), 0);
//         return;
//     }

//     // Check if user has operator privileges
//     if (!channel->isOperator(client) && channel->getCreator() != client) {
//         std::string response = ": 482 ";
//         response += client.getNickname();
//         response += " ";
//         response += channelName;
//         response += " :You're not channel operator\r\n";
//         send(client_fd, response.c_str(), response.size(), 0);
//         return;
//     }

//     // Handle mode changes
//     std::string modeStr = tokens[2];
//     if (modeStr[0] != '+' && modeStr[0] != '-') {
//         std::string response = ": 472 ";
//         response += client.getNickname();
//         response += " ";
//         response += modeStr[0];
//         response += " :is unknown mode char to me\r\n";
//         send(client_fd, response.c_str(), response.size(), 0);
//         return;
//     }

//     // Process each mode character
//     char sign = modeStr[0];
//     size_t param_index = 3;
//     std::string new_mode_str = "+";  // Start with + for new modes
    
//     for (size_t i = 1; i < modeStr.length(); i++) {
//         if (modeStr[i] == '+' || modeStr[i] == '-') {
//             sign = modeStr[i];
//             continue;
//         }

//         switch (modeStr[i]) {
//             case 'o':
//                 if (sign == '+' && tokens.size() <= param_index) {
//                     std::string response = ": 461 ";
//                     response += client.getNickname();
//                     response += " MODE :Not enough parameters\r\n";
//                     send(client_fd, response.c_str(), response.size(), 0);
//                     return;
//                 }
//                 if (sign == '+') {
//                     std::string target_nick = tokens[param_index++];
//                     for (size_t j = 0; j < channel_clients.size(); j++) {
//                         if (channel_clients[j].getNickname() == target_nick) {
//                             channel->addOperator(channel_clients[j]);
//                             break;
//                         }
//                     }
//                 }
//                 channel->setMode('o', (sign == '+'));
//                 if (sign == '+') new_mode_str += 'o';
//                 break;

//             case 'k':
//                 if (sign == '+' && tokens.size() <= param_index) {
//                     std::string response = ": 461 ";
//                     response += client.getNickname();
//                     response += " MODE :Not enough parameters\r\n";
//                     send(client_fd, response.c_str(), response.size(), 0);
//                     return;
//                 }
//                 if (sign == '+') {
//                     int key = atoi(tokens[param_index++].c_str());
//                     channel->set_key(key);
//                 } else {
//                     channel->set_key(-1);
//                 }
//                 channel->setMode('k', (sign == '+'));
//                 if (sign == '+') new_mode_str += 'k';
//                 break;

//             case 'l':
//                 if (sign == '+' && tokens.size() <= param_index) {
//                     std::string response = ": 461 ";
//                     response += client.getNickname();
//                     response += " MODE :Not enough parameters\r\n";
//                     send(client_fd, response.c_str(), response.size(), 0);
//                     return;
//                 }
//                 if (sign == '+') {
//                     std::string limit_str = tokens[param_index++];
//                     bool is_number = true;
//                     for (size_t j = 0; j < limit_str.length(); j++) {
//                         if (!isdigit(limit_str[j])) {
//                             is_number = false;
//                             break;
//                         }
//                     }
//                     if (!is_number) {
//                         std::string response = ": 461 ";
//                         response += client.getNickname();
//                         response += " MODE :Not enough parameters\r\n";
//                         send(client_fd, response.c_str(), response.size(), 0);
//                         return;
//                     }
//                     int limit = atoi(limit_str.c_str());
//                     if (channel->get_clients_size() > limit) {
//                         std::string response = ": 471 ";
//                         response += client.getNickname();
//                         response += " ";
//                         response += channelName;
//                         response += " :Cannot join channel (+l)\r\n";
//                         send(client_fd, response.c_str(), response.size(), 0);
//                         return;
//                     }
//                     channel->setMaxClients(limit);
//                 } else {
//                     channel->removeUserLimit();
//                 }
//                 channel->setMode('l', (sign == '+'));
//                 if (sign == '+') new_mode_str += 'l';
//                 break;

//             case 'i':
//                 channel->setInviteOnly(sign == '+');
//                 channel->setMode('i', (sign == '+'));
//                 if (sign == '+') new_mode_str += 'i';
//                 break;

//             case 't':
//                 channel->setTopicRestricted(sign == '+');
//                 channel->setMode('t', (sign == '+'));
//                 if (sign == '+') new_mode_str += 't';
//                 break;

//             default:
//                 std::string response = ": 472 ";
//                 response += client.getNickname();
//                 response += " ";
//                 response += modeStr[i];
//                 response += " :is unknown mode char to me\r\n";
//                 send(client_fd, response.c_str(), response.size(), 0);
//                 return;
//         }
//     }

//     channel->updateModeString();
    
//     // Prepare mode response
//     std::string response = ":";
//     response += client.getNickname();
//     response += "!";
//     if (!client.getUsername().empty()) {
//         response += client.getUsername();
//     }
//     response += " MODE ";
//     response += channelName;
//     response += " ";
//     response += modeStr;
//     if (tokens.size() > 3) {
//         response += " ";
//         response += tokens[3];
//     }
//     response += "\r\n";
    
//     // Broadcast mode change to all channel members
//     broadcastToChannel(*channel, response);
// }

// Channel* Server::getChannel(const std::string& channelName) {
//     for (size_t i = 0; i < channels.size(); i++) {
//         if (channels[i].get_name() == channelName) {
//             return &channels[i];
//         }
//     }
//     return NULL;
// }

// void Server::broadcastToChannel(const Channel& channel, const std::string& message) {
//     const std::vector<Client>& clients = channel.get_clients();
//     for (size_t i = 0; i < clients.size(); i++) {
//         send(clients[i].getFd(), message.c_str(), message.size(), 0);
//     }
// }