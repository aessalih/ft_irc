#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>

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
		~Client();
		void setNickname(const std::string& nickname);
		void setUsername(const std::string& username);
		void setIsRegestered(const bool& isRegistered);
		void setHavePass(const bool& have_pass);
		const	int& getFd(void) const;
		const std::string& getNickname(void) const;
		const std::string& getUsername(void) const;
		const bool& getIsRegistered(void) const;
		const bool& getHavePass(void) const;
		bool operator==(const Client& obj) const;
		bool operator!=(const Client& obj) const;
};

#endif
