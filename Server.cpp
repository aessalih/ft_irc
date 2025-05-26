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
		std::cerr << "SOCKET FUNCTION FAILS\n";
		return -1;
	}
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		std::cerr << "SET SOCKET OPRION FAILS\n";
		return -1;
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(this->port);
	if (bind(sockfd, (struct sockaddr*)&address, addrlen) == -1) {
		std::cerr << "BIND FUNCTION FAILS\n";
		return -1;
	}
	if (listen(sockfd, 3) == -1) {
		std::cerr << "LISTEN FAILS\n";
		return -1;
	}
	server_sockfd.fd = sockfd;
	server_sockfd.events = POLLIN;
	server_sockfd.revents = 0;
	fds.push_back(server_sockfd);
	std::cout << "SERVER START ON PORT : " << this->port << std::endl;
	return 0;
}

int Server::run() {
	while (true) {
		int	poll_count = poll(fds.data(), fds.size(), -1);
		if (poll_count < 0) {
			std::cerr << "POLL FUNCTION FAILS\n";
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
		std::cerr << "ERROR ACCEPTING CONNECTION\n";
		return -1;
	}
	newFd.fd = accept_fd;
	newFd.events = POLLIN;
	newFd.revents = 0;
	fds.push_back(newFd);
	send(newFd.fd,
		"you're connected to the server. To complete your registration, enter the server password and add your username and nickname!\n",
		126, 0);
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
		return ;
	}
	// check if client is registered
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
		std::cout << "Client registered\n";
		clients[i - 1].setIsRegestered(true);
		send(client_fd, "You complete your registration\n", 32, 0);
		return ;
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
}

int	Server::check_password(char *buffer, int fd) {
	char *pass = strtok(buffer, " ");
	std::string	password;

	if (strcmp(pass, "pass") != 0)
		return 0;
	pass = strtok(NULL, " ");
	password = pass;
	password.erase(password.find_last_not_of("\n") + 1);
	if (password == this->password)
		return 1;
	send(fd, "Enter server password: pass <password>\n", 40, 0);
	return 0;
}

int	Server::check_names(std::vector<Client> &clients, size_t i, char *buffer, int fd) {
	char *token = strtok(buffer, " ");
	size_t	j = 0;

	std::string name = token;
	name.erase(name.find_last_not_of("\n") + 1);
	if (name == "nick") {
		token = strtok(NULL, " ");
		if (!token) {
			send(fd, "You have to enter the right nickname(4-16 character)\n", 54, 0);
			return 0;
		}
		name = token;
		name.erase(name.find_last_not_of("\n") + 1);
		if (name.length() < 4 && name.length() > 16) {
			send(fd, "You have to enter the right nickname(4-16 character)\n", 54, 0);
			return 0;
		}
		while (j < clients.size()) {
			if (clients[j].getNickname() == name) {
				send(fd, "This nickname is used!\n", 24, 0);
				return 0;
			}
			j++;
		}
		clients[i].setNickname(name);
		if (!clients[i].getUsername().empty())
			return 1;
		return 0;
	}
	else if (name == "user") {
		token = strtok(NULL, " ");
		if (!token) {
			send(fd, "You have to enter the right username(4-16 character)\n", 54, 0);
			return 0;
		}
		name = token;
		name.erase(name.find_last_not_of("\n") + 1);
		if (name.length() < 4 && name.length() > 16) {
			send(fd, "You have to enter the right username(4-16 character)\n", 54, 0);
			return 0;
		}
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

std::vector<std::string> split(const std::string &s){
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	bool inWord = false;
	std::string currentWord;

	for (size_t i = 0; i < s.length(); i++) {
		if (s[i] == ' ' || s[i] == '\t' || s[i] == '\n') {
			if (inWord) {
				tokens.push_back(currentWord);
				currentWord.clear();
				inWord = false;
			}
		} else {
			currentWord += s[i];
			inWord = true;
		}
	}

	if (inWord && !currentWord.empty()) {
		tokens.push_back(currentWord);
	}

	return tokens;
}