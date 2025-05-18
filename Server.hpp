#ifndef _SERVER_HPP_
# define _SERVER_HPP_

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
		// fctnl();
		// poll();
		// 10 server
		// 5p => channel 1 
		// 1 => 1
		// privmsg #chanl ashdflasjkfjas flkasj fdkasjdfalk
		// privmsg azedine lkasjdflkasjf
		// std::vector<int> channel_client
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
};
std::vector<std::string> split(const std::string &s);
int play(int fd);

#endif

