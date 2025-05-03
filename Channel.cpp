#include "Channel.hpp"

Channel::Channel(std::string name, int key) : name(name), key(key), topic(""), mode(""), current_clients(0), max_client(-1),
	invite_only(false), topic_restricted(false) {}

Channel::Channel(const Channel &other) {
	*this = other;
}

Channel &Channel::operator=(const Channel &other) {
	if (this != &other) {
		name = other.name;
		key = other.key;
		topic = other.topic;
		mode = other.mode;
		current_clients = other.current_clients;
		max_client = other.max_client;
		clients = other.clients;
		priveleged_client = other.priveleged_client;
		invite_only = other.invite_only;
		topic_restricted = other.topic_restricted;
	}
	return *this;
}

Channel::~Channel() {
}

void Channel::set_topic(std::string new_topic) {
	topic = new_topic;
}

std::string Channel::get_topic() const {
	return topic;
}

void Channel::set_mode(std::string new_mode) {
	mode = new_mode;
}

std::string Channel::get_mode() const {
	return mode;
}

void Channel::set_max_clients(int max) {
	max_client = max;
}

void Channel::remove_user_limit() {
	max_client = -1;
}

std::string Channel::get_name() const {
	return name;
}

int Channel::get_key() const {
	return key;
}

void Channel::set_key(int new_key) {
	key = new_key;
}

void Channel::add_client(Client &client) {
	clients.push_back(client);
	current_clients++;
}

void Channel::delete_client(Client &client) {
	for (size_t i = 0; i < clients.size(); i++) {
		if (clients[i] == client) {
			clients.erase(clients.begin() + i);
			current_clients--;
			break;
		}
	}
}

size_t Channel::get_clients_size() const {
	return clients.size();
}

const Client &Channel::get_client(size_t index) const {
	return clients[index];
}

const std::vector<Client> &Channel::get_clients() const {
	return clients;
}

void Channel::setInviteOnly(bool value) { invite_only = value; }
bool Channel::isInviteOnly() const { return invite_only; }

void Channel::setTopicRestricted(bool value) { topic_restricted = value; }
bool Channel::isTopicRestricted() const { return topic_restricted; }

void Channel::setKey(int new_key) { key = new_key; }
void Channel::removeKey() { key = -1; }
bool Channel::hasKey() const { return key != -1; }

void Channel::setMaxClients(int max) { max_client = max; }
void Channel::removeUserLimit() { max_client = -1; }
bool Channel::hasUserLimit() const { return max_client != -1; }

void Channel::addOperator(const Client& client) {
	for (size_t i = 0; i < priveleged_client.size(); ++i) {
		if (priveleged_client[i] == client)
			return;
	}
	priveleged_client.push_back(client);
}
void Channel::removeOperator(const Client& client) {
	for (size_t i = 0; i < priveleged_client.size(); ++i) {
		if (priveleged_client[i] == client) {
			priveleged_client.erase(priveleged_client.begin() + i);
			break;
		}
	}
}
bool Channel::isOperator(const Client& client) const {
	for (size_t i = 0; i < priveleged_client.size(); ++i) {
		if (priveleged_client[i] == client)
			return true;
	}
	return false;
}
