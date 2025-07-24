#include "Server.hpp"
#include <sstream>

// canonical form and parameterize constructor definition
Server::Server() {
	this->opt = 1;
	this->addrlen = sizeof(address);
	memset(buffer, 0, 512);
	this->port = 8080;
	if (this->port == -1)
		exit (EXIT_FAILURE);
	this->password = "1234";
}

Server::Server(char **av) {
	flag = 1;
	this->opt = 1;
	this->addrlen = sizeof(address);
	memset(buffer, 0, 512);
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
		close(sockfd);
		std::cerr << "\033[1;31mSET SOCKET OPRION FAILS\033[0m\n";
		return -1;
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(this->port);
	if (bind(sockfd, (struct sockaddr*)&address, addrlen) == -1) {
		std::cerr << ERROR << std::endl;
		close(sockfd);
		std::cerr << "\033[1;31mBIND FUNCTION FAILS\033[0m\n";
		return -1;
	}
	if (listen(sockfd, 3) == -1) {
		std::cerr << ERROR << std::endl;
		close(sockfd);
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

void Server::close_fds() {
	std::cout << std::endl;
	std::cout << std::endl;
	for (size_t i = 0; i < clients.size(); i++) {
		close(clients[i].getFd());
	}
	close(sockfd);
}

void Server::stopServer(int signalNum) {
	(void) signalNum;
	flag = 0;
}

int Server::run() {
	std::signal(SIGINT, stopServer);
	std::signal(SIGTERM, stopServer);
	std::signal(SIGPIPE, SIG_IGN);
	std::signal(SIGQUIT, SIG_IGN);
	while (this->flag) {
		int	poll_count = poll(fds.data(), fds.size(), -1);
		if (poll_count < 0) {
			if (this->flag == 0)
				break ;
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
				std::cout << "\033[1;32mClient " << clients[i - 1].getNickname() << " disconnected\033[0m" << "\n";
				removeClient(fds[i].fd);
				close(fds[i].fd);
				fds.erase(fds.begin() + i);
				clients.erase(clients.begin() + i - 1);
				i--;
			}
		}
	}
	close_fds();
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
	char ip_str[INET_ADDRSTRLEN];
	std::string ip;
	if (inet_ntop(AF_INET, &clientAddr.sin_addr, ip_str, INET_ADDRSTRLEN) != NULL) {
		ip = ip_str;
	} else {
		ip = "unknown";
	}
	Client client(newFd.fd, ip);
	clients.push_back(client);
	return (1);
}

void Server::handleClientMessage(size_t i) {
	memset(buffer, 0, 512);

	int client_fd = fds[i].fd;
	int bytes = recv(client_fd, buffer, 512, 0);
	if (bytes <= 0) {
		std::cout << "\033[1;32mClient " << clients[i - 1].getNickname() << " disconnected\033[0m" << "\n";
		removeClient(client_fd);
		close(client_fd);
		fds.erase(fds.begin() + i);
		clients.erase(clients.begin() + i - 1);
		return ;
	}
	std::string checkBuffer = buffer;
	if (checkBuffer.find("\r\n") == std::string::npos) {
		clients[i - 1].buffer += checkBuffer;
		return ;
	}
	clients[i - 1].buffer += checkBuffer;
	if (clients[i - 1].buffer.size() > 512) {
		std::cout << "\033[1;31mBUFFER OVERFLOWS...TRY AGAIN \033[0m" << std::endl;
		clients[i - 1].buffer = "";
		return ;
	}
	for (size_t j = 0; j < clients[i - 1].buffer.size(); j++) {
		buffer[j] = clients[i - 1].buffer[j];
	}
	clients[i - 1].buffer = "";
	std::cout << buffer << std::endl;
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
	for (size_t i = 0; i < clients.size(); i++) {
		std::cout << "client" << i << " = " << clients[i].getNickname() << std::endl;
	}

	std::string msg(buffer);
	std::vector<std::string> tokens = split(msg);
	if (tokens.empty())
		return;
	std::string cmd = toUpperCase(tokens[0]);
	if (cmd == "JOIN") {
        handleJoin(i, client_fd, tokens);
    }
    else if (cmd == "PRIVMSG") {
        handlePrivmsg(i, client_fd, tokens, msg);
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
	else if (cmd == "QUIT") {
		std::cout << "\033[1;32mClient " << clients[i - 1].getNickname() << " disconnected\033[0m" << "\n";
		removeClient(client_fd);
		close(client_fd);
		fds.erase(fds.begin() + i);
		// We have to delete the client from channels.
		clients.erase(clients.begin() + i - 1);
		return ;
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
	password = toUpperCase(password);
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
	name = toUpperCase(name);
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
		if (name.size() > 9) {
			response = ":irc 432 * "+ name +" : :Too long nickname\r\n";
			send(fd, response.c_str(), response.size(), 0);
			return 0;
		}
		clients[i].setNickname(name);
		if (!clients[i].getUsername().empty())
			return 1;
		return 0;
	}
	else if (name == "USER") {
		std::vector<std::string> a = split1(newBuffer);
		if (split1(newBuffer).size() != 5) {
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


// std::vector<std::string> split(const std::string &s) {
//     std::vector<std::string> tokens;
//     std::string currentWord;
//     bool inWord = false;

//     for (size_t i = 0; i < s.length(); ++i) {
//         char c = s[i];
//         if (c == ' ' || c == '\t' || c == '\n') {
//             if (inWord) {  // End of a word
//                 tokens.push_back(currentWord);
//                 currentWord.clear();
//                 inWord = false;
//             }
//         } else {
//             currentWord += c;
//             inWord = true;
//         }
//     }

//     // Add the last word if it exists
//     if (inWord) {
//         tokens.push_back(currentWord);
//     }

//     return tokens;
// }


std::vector<std::string> split(const std::string &s) {
	std::istringstream			stream(s);
	std::vector<std::string>	tokens;
	std::string					token;

	// if ':'

	while (stream >> token) {
		tokens.push_back(token);
		token.clear();
	}
	return tokens;
}

std::string	Server::toUpperCase(std::string s) {
	std::string newStr = "";
	for(size_t i = 0; i < s.size(); i++) {
		newStr += (char)toupper(s[i]);
	}
	return newStr;
}

std::string Server::toLowerCase(std::string s) {
    std::string newStr = "";
    for (size_t i = 0; i < s.size(); i++) {
        newStr += (char)tolower(s[i]);
    }
    return newStr;
}

void Server::removeClient(int fd) {
	for (size_t i = 0; i < channels.size(); i++) {
		for (size_t j = 0; j < channels[i].clients.size(); j++) {
			if (channels[i].clients[j].getFd() == fd) {
				channels[i].clients.erase(channels[i].clients.begin() + j);
				break ;
			}
		}
	}
}