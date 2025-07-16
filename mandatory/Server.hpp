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
		
		//join
    	bool isValidChannelName(const std::string &name);
    	Channel	*findOrCreateChannel(const std::string &name, const std::string &key, Client &creator);
    	bool canClientJoinChannel(Channel *channel, Client &client, const std::string &name, const std::string &key, int client_fd);
    	bool isClientInvited(Channel *channel, Client &client);
    	bool isClientAlreadyInChannel(Channel *channel, Client &client, int client_fd, const std::string &name);
    	void notifyJoin(Channel *channel, Client &client, const std::string &name);
    	void sendTopicIfExists(Channel *channel, int client_fd, const std::string &nickname, const std::string &name);
    	void sendNamesList(Channel *channel, int client_fd, const std::string &nickname, const std::string &name);

		//invite 
		Client *findGlobalClientByNick(const std::string &target_nick);
    	void sendInviteMessages(Channel *channel, Client &inviter, Client *target_client, const std::string &channel_name, const std::string &target_nick, int inviter_fd);

		//privmsg
		std::string extractMessage(const std::vector<std::string> &tokens);
    	void cleanMessage(std::string &msg);
    	bool isChannelTarget(const std::string &target);
    	void handleChannelPrivmsg(const std::string &target, const std::string &msg, Client &sender, int client_fd);
    	void handleUserPrivmsg(const std::string &target, const std::string &msg, Client &sender, int client_fd);
    	// Channel *findChannelByName(const std::string &name);
    	// bool isUserInChannel(Channel *channel, Client &client);
    	void sendPrivmsgToChannel(Channel *channel, const std::string &msg, Client &sender);
    	void sendError(int client_fd, int code, const std::string &nickname, const std::string &target, const std::string &msg);

		//kick
		 bool isValidKickChannelName(const std::string &name);
    	bool isUserInChannel(Channel *channel, Client &client);
    	Client *findClientInChannel(Channel *channel, const std::string &target_nick);
    	bool isUserOperatorOrCreator(Channel *channel, Client &client);
    	void sendKickMessage(Channel *channel, Client &kicker, Client &target, const std::string &channel_name, const std::string &target_nick);

		//topic
		Channel *findChannelByName(const std::string &name);
    	// bool isUserInChannel(Channel &channel, Client &client);
    	// void sendError(int client_fd, int code, const std::string &nickname, const std::string &channel, const std::string &message);
   		void sendCurrentTopic(int client_fd, Client &client, Channel &channel, const std::string &channel_name);
    	void setNewTopicAndNotify(Client &client, Channel &channel, const std::string &new_topic, const std::string &channel_name);

        // MODE command
        void handleMode(size_t i, int client_fd, const std::vector<std::string> &tokens);
        void handleModeInvite(Channel* channel, bool adding, int client_fd, const std::string& nickname, const std::string &channel_name);
        void handleModeLimit(Channel *channel, bool adding, const std::string &param, int client_fd, const std::string &nickname, const std::string &channel_name);
        void handleModeTopic(Channel *channel, bool adding, int client_fd, const std::string &nickname, const std::string &channel_name);
        void handleModeKey(Channel *channel, bool adding, const std::string &param, int client_fd, const std::string &nickname, const std::string &channel_name);
        void handleModeOperator(Channel *channel, bool adding, const std::string &param, int client_fd, const std::string &nickname, const std::string &channel_name);

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

		void handleJoin(size_t i, int client_fd, const std::vector<std::string> &tokens);
    	void handlePrivmsg(size_t i, int client_fd, const std::vector<std::string> &tokens, const std::string &msg);
		void handleKick(size_t i, int client_fd, const std::vector<std::string> &tokens);
   		void handleInvite(size_t i, int client_fd, const std::vector<std::string> &tokens);
   		void handleTopic(size_t i, int client_fd, const std::vector<std::string> &tokens);
		
};
std::vector<std::string> split(const std::string &s);
std::vector<std::string> split1(const std::string &s);

#endif

