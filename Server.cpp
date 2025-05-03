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

// Methods definition
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
	// Check if client is registered
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
		send(client_fd, "You complete your registration\n", 32, 0);
		return ;
	}

	// Parse the message
	std::string msg(buffer);
	std::vector<std::string> tokens = split(msg, ' ');
	
	std::string cmd = tokens[0];
	if (!cmd.empty() && cmd[cmd.size() - 1] == '\n') {
		cmd.erase(cmd.size() - 1);
	}	

	if (cmd == "JOIN") {
		if (tokens.size() < 2) {
			std::string error_msg = "461 JOIN :Not enough parameters\n";
			send(client_fd, error_msg.c_str(), error_msg.length(), 0);
			return;
		}

		// Parse channel name and key
		std::string channel_name = tokens[1];
		if (!channel_name.empty() && channel_name[channel_name.size() - 1] == '\n') {
			channel_name.erase(channel_name.size() - 1);
		}	
		std::cout << "[" << channel_name << "]" <<std::endl;	
		std::string key = "";
		if (tokens.size() >= 3)
			key = tokens[2];

		// Validate channel name
		if (channel_name[0] != '#') {
			std::string error_msg = "403 " + channel_name + " :No such channel\n";
			send(client_fd, error_msg.c_str(), error_msg.length(), 0);
			return;
		}

		// Check if channel exists
		bool channel_exists = false;
		Channel *target_channel = NULL;
		
		for (size_t j = 0; j < channels.size(); j++) {
			if (channels[j].get_name() == channel_name) {
				channel_exists = true;
				target_channel = &channels[j];
				break;
			}
		}

		if (!channel_exists) {
			// Create new channel
			int channel_key = -1;
			if (!key.empty()) {
				std::istringstream iss(key);
				iss >> channel_key;
			}
			Channel new_channel(channel_name, channel_key);
			// Add the creator as operator
			new_channel.addOperator(clients[i - 1]);
			channels.push_back(new_channel);
			target_channel = &channels.back();
		}

		// Add client to channel
		if (target_channel) {
			// Check if channel need a key
			int channel_key = target_channel->get_key();
			int provided_key = -1;
			if (!key.empty()) {
				std::istringstream iss(key);
				iss >> provided_key;
			}
			
			if (channel_key != -1 && (key.empty() || channel_key != provided_key)) {
				std::string error_msg = "475 " + channel_name + " :Cannot join channel (+k)\n";
				send(client_fd, error_msg.c_str(), error_msg.length(), 0);
				return;
			}

			// Add client to channel
			target_channel->add_client(clients[i - 1]);
			
			// Send JOIN message to all clients in channel
			std::string join_msg = ":" + clients[i - 1].getNickname() + " JOIN " + channel_name + "\n";
			const std::vector<Client> &channel_clients = target_channel->get_clients();
			for (size_t j = 0; j < channel_clients.size(); j++) {
				send(channel_clients[j].getFd(), join_msg.c_str(), join_msg.length(), 0);
			}
			
			// Send RPL_TOPIC if exists
			if (!target_channel->get_topic().empty()) {
				std::string topic_msg = "332 " + clients[i - 1].getNickname() + " " + channel_name + " :" + target_channel->get_topic() + "\n";
				send(client_fd, topic_msg.c_str(), topic_msg.length(), 0);
			}

			// Send RPL_NAMREPLY
			std::string names_msg = "353 " + clients[i - 1].getNickname() + " = " + channel_name + " :";
			const std::vector<Client> &clients_list = target_channel->get_clients();
			for (size_t j = 0; j < clients_list.size(); j++) {
				if (j != 0)
					names_msg += " ";
				names_msg += clients_list[j].getNickname();
			}
			names_msg += "\n";
			send(client_fd, names_msg.c_str(), names_msg.length(), 0);

			// Send RPL_ENDOFNAMES
			std::string end_names_msg = "366 " + clients[i - 1].getNickname() + " " + channel_name + " :End of /NAMES list\n";
			send(client_fd, end_names_msg.c_str(), end_names_msg.length(), 0);
		}
	}
	else if (tokens[0] == "PRIVMSG") {
		// std::cout << tokens[0] << "\n";
		// std::cout << tokens[1] << "\n";
		// std::cout << tokens[2] << "\n";
		if (tokens.size() < 3) {
			std::string error_msg = "461 PRIVMSG :Not enough parameters\n";
			send(client_fd, error_msg.c_str(), error_msg.length(), 0);
			return;
		}

		std::string target = tokens[1];
		std::string message;

		// Combine all remaining tokens into the message
		for (size_t j = 2; j < tokens.size(); j++) {
			if (j > 2)
				message += " ";
			message += tokens[j];
		}
		if (!message.empty() && message[message.size() - 1] == '\n') {
			message.erase(message.size() - 1);
		}
		//Remove quotes if present
		if (message[0] == '"' && message[message.length() - 1] == '"') {
			message = message.substr(1, message.length() - 2);
			std::cout << message << std::endl;
		}

		// Check if target is a channel
		if (target[0] == '#') {
			// Find the channel
			Channel *target_channel = NULL;
			for (size_t j = 0; j < channels.size(); j++) {
				std::cout << channels[j].get_name() << "choof \n";
				if (channels[j].get_name() == target) {
					target_channel = &channels[j];
					break;
				}
			}

			if (!target_channel) {
				std::string error_msg = "403 " + target + " :No such channel\n";
				send(client_fd, error_msg.c_str(), error_msg.length(), 0);
				return;
			}

			// Check if sender is in the channel
			bool is_in_channel = false;
			const std::vector<Client> &channel_clients = target_channel->get_clients();
			for (size_t j = 0; j < channel_clients.size(); j++) {
				if (channel_clients[j] == clients[i - 1]) {
					is_in_channel = true;
					break;
				}
			}

			if (!is_in_channel) {
				std::string error_msg = "404 " + target + " :Cannot send to channel\n";
				send(client_fd, error_msg.c_str(), error_msg.length(), 0);
				return;
			}

			// Send message to all clients in channel
			std::string privmsg = ":" + clients[i - 1].getNickname() + " PRIVMSG " + target + " :" + message + "\n";
			for (size_t j = 0; j < channel_clients.size(); j++) {
				if (!(channel_clients[j] == clients[i - 1])) { // Don't send to self
					send(channel_clients[j].getFd(), privmsg.c_str(), privmsg.length(), 0);
				}
			}
		}
		else {
			// Target is a user
			bool user_found = false;
			for (size_t j = 0; j < clients.size(); j++) {
				if (clients[j].getNickname() == target) {
					user_found = true;
					std::string privmsg = ":" + clients[i - 1].getNickname() + " PRIVMSG " + target + " :" + message + "\n";
					send(clients[j].getFd(), privmsg.c_str(), privmsg.length(), 0);
					break;
				}
			}

			if (!user_found) {
				std::string error_msg = "401 " + target + " :No such nick\n";
				send(client_fd, error_msg.c_str(), error_msg.length(), 0);
			}
		}
	}
	else if (cmd == "KICK") {
		if (tokens.size() < 3) {
			std::string error_msg = "461 KICK :Not enough parameters\n";
			send(client_fd, error_msg.c_str(), error_msg.length(), 0);
			return;
		}

		std::string channel_name = tokens[1];
		std::string target_nick = tokens[2];
		if (!channel_name.empty() && channel_name[channel_name.size() - 1] == '\n') {
			channel_name.erase(channel_name.size() - 1);
		}
		if (!target_nick.empty() && target_nick[target_nick.size() - 1] == '\n') {
			target_nick.erase(target_nick.size() - 1);
		}

		// Validate channel name
		if (channel_name[0] != '#') {
			std::string error_msg = "403 " + channel_name + " :No such channel\n";
			send(client_fd, error_msg.c_str(), error_msg.length(), 0);
			return;
		}

		// Find the channel
		Channel *target_channel = NULL;
		for (size_t j = 0; j < channels.size(); j++) {
			if (channels[j].get_name() == channel_name) {
				target_channel = &channels[j];
				break;
			}
		}

		if (!target_channel) {
			std::string error_msg = "403 " + channel_name + " :No such channel\n";
			send(client_fd, error_msg.c_str(), error_msg.length(), 0);
			return;
		}

		// Check if kicker is in the channel
		bool kicker_in_channel = false;
		const std::vector<Client> &channel_clients = target_channel->get_clients();
		for (size_t j = 0; j < channel_clients.size(); j++) {
			if (channel_clients[j] == clients[i - 1]) {
				kicker_in_channel = true;
				break;
			}
		}

		if (!kicker_in_channel) {
			std::string error_msg = "442 " + channel_name + " :You're not on that channel\n";
			send(client_fd, error_msg.c_str(), error_msg.length(), 0);
			return;
		}

		// Find target client in channel
		bool target_found = false;
		Client *target_client = NULL;
		for (size_t j = 0; j < channel_clients.size(); j++) {
			if (channel_clients[j].getNickname() == target_nick) {
				target_found = true;
				target_client = const_cast<Client*>(&channel_clients[j]);
				break;
			}
		}

		if (!target_found) {
			std::string error_msg = "441 " + target_nick + " " + channel_name + " :They aren't on that channel\n";
			send(client_fd, error_msg.c_str(), error_msg.length(), 0);
			return;
		}

		// Remove client from channel
		target_channel->delete_client(*target_client);

		// Send KICK message to all clients in channel
		std::string kick_msg = ":" + clients[i - 1].getNickname() + " KICK " + channel_name + " " + target_nick + "\n";
		for (size_t j = 0; j < channel_clients.size(); j++) {
			send(channel_clients[j].getFd(), kick_msg.c_str(), kick_msg.length(), 0);
		}
	}
	else if (cmd == "INVITE") {
		if (tokens.size() < 3) {
			std::string error_msg = "461 INVITE :Not enough parameters\n";
			send(client_fd, error_msg.c_str(), error_msg.length(), 0);
			return;
		}

		std::string target_nick = tokens[1];
		std::string channel_name = tokens[2];
		if (!channel_name.empty() && channel_name[channel_name.size() - 1] == '\n') {
			channel_name.erase(channel_name.size() - 1);
		}
		if (!target_nick.empty() && target_nick[target_nick.size() - 1] == '\n') {
			target_nick.erase(target_nick.size() - 1);
		}

		// Validate channel name
		if (channel_name[0] != '#') {
			std::string error_msg = "403 " + channel_name + " :No such channel\n";
			send(client_fd, error_msg.c_str(), error_msg.length(), 0);
			return;
		}

		// Find the channel
		Channel *target_channel = NULL;
		for (size_t j = 0; j < channels.size(); j++) {
			if (channels[j].get_name() == channel_name) {
				target_channel = &channels[j];
				break;
			}
		}

		if (!target_channel) {
			std::string error_msg = "403 " + channel_name + " :No such channel\n";
			send(client_fd, error_msg.c_str(), error_msg.length(), 0);
			return;
		}

		// Check if inviter is in the channel
		bool inviter_in_channel = false;
		const std::vector<Client> &channel_clients = target_channel->get_clients();
		for (size_t j = 0; j < channel_clients.size(); j++) {
			if (channel_clients[j] == clients[i - 1]) {
				inviter_in_channel = true;
				break;
			}
		}

		if (!inviter_in_channel) {
			std::string error_msg = "442 " + channel_name + " :You're not on that channel\n";
			send(client_fd, error_msg.c_str(), error_msg.length(), 0);
			return;
		}

		// Find target client in global client list
		bool target_found = false;
		Client *target_client = NULL;
		for (size_t j = 0; j < clients.size(); j++) {
			if (clients[j].getNickname() == target_nick) {
				target_found = true;
				target_client = &clients[j];
				break;
			}
		}

		if (!target_found) {
			std::string error_msg = "401 " + target_nick + " :No such nick\n";
			send(client_fd, error_msg.c_str(), error_msg.length(), 0);
			return;
		}

		// Check if target is already in the channel
		for (size_t j = 0; j < channel_clients.size(); j++) {
			if (channel_clients[j] == *target_client) {
				std::string error_msg = "443 " + target_nick + " " + channel_name + " :is already on channel\n";
				send(client_fd, error_msg.c_str(), error_msg.length(), 0);
				return;
			}
		}

		// Send INVITE message to target client
		std::string invite_msg = ":" + clients[i - 1].getNickname() + " INVITE " + target_nick + " " + channel_name + "\n";
		send(target_client->getFd(), invite_msg.c_str(), invite_msg.length(), 0);

		// Send RPL_INVITING to inviter
		std::string inviting_msg = "341 " + clients[i - 1].getNickname() + " " + target_nick + " " + channel_name + "\n";
		send(client_fd, inviting_msg.c_str(), inviting_msg.length(), 0);
	}
	else if (cmd == "TOPIC") {
		if (tokens.size() < 2) {
			std::string error_msg = "461 TOPIC :Not enough parameters\n";
			send(client_fd, error_msg.c_str(), error_msg.length(), 0);
			return;
		}

		std::string channel_name = tokens[1];
		Channel* target_channel = NULL;

		if (!channel_name.empty() && channel_name[channel_name.size() - 1] == '\n') {
			channel_name.erase(channel_name.size() - 1);

		// Find the channel
		for (size_t j = 0; j < channels.size(); j++) {
			if (channels[j].get_name() == channel_name) {
				target_channel = &channels[j];
				break;
			}
		}

		if (!target_channel) {
			std::string error_msg = "403 " + channel_name + " :No such channel\n";
			send(client_fd, error_msg.c_str(), error_msg.length(), 0);
			return;
		}

		// Check if user is in the channel
		bool user_in_channel = false;
		const std::vector<Client>& channel_clients = target_channel->get_clients();
		for (size_t j = 0; j < channel_clients.size(); j++) {
			if (channel_clients[j] == clients[i - 1]) {
				user_in_channel = true;
				break;
			}
		}

		if (!user_in_channel) {
			std::string error_msg = "442 " + channel_name + " :You're not on that channel\n";
			send(client_fd, error_msg.c_str(), error_msg.length(), 0);
			return;
		}

		// If no topic provided, show current topic
		if (tokens.size() == 2) {
			std::string topic = target_channel->get_topic();
			if (topic.empty()) {
				std::string msg = "331 " + channel_name + " :No topic is set\n";
				send(client_fd, msg.c_str(), msg.length(), 0);
			} else {
				std::string msg = "332 " + channel_name + " :" + topic + "\n";
				send(client_fd, msg.c_str(), msg.length(), 0);
			}
			return;
		}

		// Setting new topic
		if (target_channel->isTopicRestricted() && !target_channel->isOperator(clients[i - 1])) {
			std::string error_msg = "482 " + channel_name + " :You're not channel operator\n";
			send(client_fd, error_msg.c_str(), error_msg.length(), 0);
			return;
		}

		// Combine remaining tokens for topic
		std::string new_topic;
		for (size_t j = 2; j < tokens.size(); j++) {
			if (j > 2) new_topic += " ";
			new_topic += tokens[j];
		}
		

		target_channel->set_topic(new_topic);
		
		// Notify all channel members of topic change
		std::string topic_msg = ":" + clients[i - 1].getNickname() + " TOPIC " + channel_name + " :" + new_topic + "\n";
		for (size_t j = 0; j < channel_clients.size(); j++) {
			send(channel_clients[j].getFd(), topic_msg.c_str(), topic_msg.length(), 0);
		}
	}
	else if (cmd == "MODE") {
		if (tokens.size() < 3) {
			std::string error_msg = "461 MODE :Not enough parameters\n";
			send(client_fd, error_msg.c_str(), error_msg.length(), 0);
			return;
		}
		std::string channel_name = tokens[1];
		std::string mode_string = tokens[2];
		Channel* target_channel = NULL;
		for (size_t j = 0; j < channels.size(); j++) {
			if (channels[j].get_name() == channel_name) {
				target_channel = &channels[j];
				break;
			}
		}
		if (!target_channel) {
			std::string error_msg = "403 " + channel_name + " :No such channel\n";
			send(client_fd, error_msg.c_str(), error_msg.length(), 0);
			return;
		}
		// Only operators can change most modes
		if (!target_channel->isOperator(clients[i - 1])) {
			std::string error_msg = "482 " + channel_name + " :You're not channel operator\n";
			send(client_fd, error_msg.c_str(), error_msg.length(), 0);
			return;
		}
		size_t param_idx = 3;
		bool adding = true;
		for (size_t m = 0; m < mode_string.size(); ++m) {
			char mode = mode_string[m];
			if (mode == '+')
				adding = true;
			if (mode == '-')
				adding = false;
			switch (mode) {
				case 'i':
					target_channel->setInviteOnly(adding);
					break;
				case 't':
					target_channel->setTopicRestricted(adding);
					break;
				case 'k':
					if (adding) {
						if (tokens.size() > param_idx) {
							int key_val;
							std::istringstream iss(tokens[param_idx++]);
							iss >> key_val;
							target_channel->setKey(key_val);
						} else {
							std::string error_msg = "461 MODE :Not enough parameters for +k\n";
							send(client_fd, error_msg.c_str(), error_msg.length(), 0);
							return;
						}
					} else {
						target_channel->removeKey();
					}
					break;
				case 'o':
					if (tokens.size() > param_idx) {
						std::string nick = tokens[param_idx++];
						Client* op_client = NULL;
						for (size_t c = 0; c < clients.size(); ++c) {
							if (clients[c].getNickname() == nick) {
								op_client = &clients[c];
								break;
							}
						}
						if (!op_client) {
							std::string error_msg = "401 " + nick + " :No such nick\n";
							send(client_fd, error_msg.c_str(), error_msg.length(), 0);
							return;
						}
						if (adding)
							target_channel->addOperator(*op_client);
						else
							target_channel->removeOperator(*op_client);
					} else {
						std::string error_msg = "461 MODE :Not enough parameters for +o/-o\n";
						send(client_fd, error_msg.c_str(), error_msg.length(), 0);
						return;
					}
					break;
				case 'l':
					if (adding) {
						if (tokens.size() > param_idx) {
							int limit_val;
							std::istringstream iss(tokens[param_idx++]);
							iss >> limit_val;
							target_channel->setMaxClients(limit_val);
						} else {
							std::string error_msg = "461 MODE :Not enough parameters for +l\n";
							send(client_fd, error_msg.c_str(), error_msg.length(), 0);
							return;
						}
					} else {
						target_channel->removeUserLimit();
					}
					break;
			}
		}
		std::string reply = "324 " + clients[i - 1].getNickname() + " " + channel_name + " " + mode_string + "\n";
		send(client_fd, reply.c_str(), reply.length(), 0);
		return;
	}
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

std::vector<std::string> split(const std::string &s, char delimiter) {
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter)) {
		tokens.push_back(token);
	}
	return tokens;
}