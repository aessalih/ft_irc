#ifndef _CLIENT_HPP_
# define _CLIENT_HPP_

#include <iostream>

class Client {
	private:
		int fd;
		std::string nickname; 
		std::string username;
		bool	have_pass;
		bool    isRegistered;
	public:
		Client(int	fd);
		Client(const Client& obj);
		Client& operator=(const Client& obj);
		void setNickname(const std::string& nickname);
		void setUsername(const std::string& username);
		void setIsRegestered(const bool& isRegistered);
		void setHavePass(const bool& have_pass);
		const	int& getFd(void);
		const std::string& getNickname(void);
		const std::string& getUsername(void);
		const bool& getIsRegistered(void);
		const bool& getHavePass(void);
		~Client();
};


#endif