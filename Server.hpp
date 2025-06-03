#ifndef _SERVER_HPP_
# define _SERVER_HPP_

# define RUNNING "\033[1;33mSERVER RUNNING... \033[0m"
# define LAUNCHED "\033[1;32mSERVER LAUNCHED \033[0m"
# define ERROR "\033[1;31mSERVER FAILED... \033[0m"

# include <iostream>
# include <sys/types.h>
# include <unistd.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <string>
# include <string.h>
# include <cstdlib>
# include <poll.h>
# include <fcntl.h>
# include <csignal>
# include <sys/socket.h>
# include <vector>
# include <sstream>
# include "Client.hpp"
# include "Channel.hpp"

class Server {
	private:
		struct sockaddr_in	address;
		int	sockfd;
		int	accept_fd;
		int	opt;
		int	port;
		int	addrlen;
		struct pollfd	server_sockfd;
		char	buffer[1024];
		std::string	password;
		std::vector<Channel> channels;
		std::vector<Client>	clients;
		std::vector<struct pollfd> fds;

		int	parse_port(char *port);

	public:
		// canonical form and parameterize constructor
		Server();
		Server(char **av);
		Server(const Server& obj);
		Server&	operator=(const Server& obj);
		~Server();

		// methods
		int	init();
		int run();
		int handleNewClients();
		void handleClientMessage(size_t i);
		int	check_password(char *buffer, int fd);
		int	check_names(std::vector<Client> &clients, size_t i, char *buffer, int fd);

		void handleJoin(size_t i, int client_fd, const std::vector<std::string>& tokens);
    	void handlePrivmsg(size_t i, int client_fd, const std::vector<std::string>& tokens);
		void handleKick(size_t i, int client_fd, const std::vector<std::string>& tokens);
   		void handleInvite(size_t i, int client_fd, const std::vector<std::string>& tokens);
   		void handleTopic(size_t i, int client_fd, const std::vector<std::string>& tokens);
   		void handleMode(size_t i, int client_fd, const std::vector<std::string>& tokens);

		Channel* getChannel(const std::string& channelName);
		void broadcastToChannel(const Channel& channel, const std::string& message);

		// Mode handling functions
		std::string displayChannelModes(const std::string& clientNickName, const Channel& channel);
		
};
std::vector<std::string> split(const std::string &s);
std::vector<std::string> split1(const std::string &s);

#endif

