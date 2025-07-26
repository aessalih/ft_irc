#include "Client.hpp"

Client::Client(int fd, const std::string& ip) : fd(fd), ip(ip), nickname(""), username(""), have_pass(false), isRegistered(false) {
}

Client::Client(const Client& obj) : fd(obj.fd), ip(obj.ip), nickname(obj.nickname), username(obj.username),
	have_pass(obj.have_pass), isRegistered(obj.isRegistered) {
}

Client& Client::operator=(const Client& obj) {
	if (this != &obj) {
		fd = obj.fd;
		ip = obj.ip;
		buffer = obj.buffer;
		nickname = obj.nickname;
		username = obj.username;
		have_pass = obj.have_pass;
		isRegistered = obj.isRegistered;
	}
	return *this;
}

Client::~Client() {
}

void Client::setNickname(const std::string& nickname) {
	this->nickname = nickname;
}

void Client::setUsername(const std::string& username) {
	this->username = username;
}

void Client::setIsRegestered(const bool& isRegistered) {
	this->isRegistered = isRegistered;
}

void Client::setHavePass(const bool& have_pass) {
	this->have_pass = have_pass;
}

const int& Client::getFd(void) const {
	return fd;
}

const std::string& Client::getNickname(void) const {
	return nickname;
}

const std::string& Client::getUsername(void) const {
	return username;
}

const bool& Client::getIsRegistered(void) const {
	return isRegistered;
}

const bool& Client::getHavePass(void) const {
	return have_pass;
}

const std::string &Client::getIp(void) const {
	return ip;
}

bool Client::operator==(const Client& obj) const {
	return (fd == obj.fd && nickname == obj.nickname && username == obj.username);
}

bool Client::operator!=(const Client& obj) const {
	return (fd != obj.fd && nickname != obj.nickname && username != obj.username); 
}
