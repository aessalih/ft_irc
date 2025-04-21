#include "Client.hpp"

Client::Client(int  fd) {
    this->fd = fd;
    this->nickname = ""; 
    this->username = "";
    this->isRegistered = false;
    this->have_pass = false;
}

Client::Client(const Client& obj) {
    this->fd = obj.fd;
    this->nickname = obj.nickname;
    this->username = obj.username;
    this->isRegistered = obj.isRegistered;
    this->have_pass = obj.have_pass;
}

Client&	Client::operator=(const Client& obj) {
    if (this != &obj) {
        this->fd = obj.fd;
        this->nickname = obj.nickname;
        this->username = obj.username;
        this->isRegistered = obj.isRegistered;
        this->have_pass = obj.have_pass;
    }
    return (*this);
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

const int&  Client::getFd() {
    return (this->fd);
}

const std::string& Client::getNickname(void) {
    return (this->nickname);
}

const std::string& Client::getUsername(void) {
    return (this->username);
}

const bool& Client::getIsRegistered(void) {
    return (this->isRegistered);
}

const bool& Client::getHavePass(void) {
    return (this->have_pass);
}

Client::~Client() {
	
}