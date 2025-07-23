#include "Channel.hpp"

Channel::Channel() : name(""), key(""), topic(""), mode(""), current_clients(0), max_client(-1),
	creator(-1, ""), invite_only(false), topic_restricted(false) {
	channel_mode.i = -1;
	channel_mode.t = -1;
	channel_mode.k = -1;
	channel_mode.o = -1;
	channel_mode.l = -1;
	topic_set_time = 0;
}

Channel::Channel(std::string name, std::string key) : name(name), key(key), topic(""), mode(""), current_clients(0), max_client(-1),
	creator(-1, ""), invite_only(false), topic_restricted(false) {
	channel_mode.i = -1;
	channel_mode.t = -1;
	channel_mode.k = -1;
	channel_mode.o = -1;
	channel_mode.l = -1;
	topic_set_time = 0;
}

Channel::Channel(const Channel &other) : name(other.name), key(other.key), topic(other.topic), 
	mode(other.mode), current_clients(other.current_clients), max_client(other.max_client),
	creator(other.creator), channel_mode(other.channel_mode) {
	clients = other.clients;
	priveleged_client = other.priveleged_client;
	topic_set_time = other.topic_set_time;
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
		creator = other.creator;
		invite_only = other.invite_only;
		topic_restricted = other.topic_restricted;
		channel_mode = other.channel_mode;
		topic_set_time = other.topic_set_time;
	}
	return *this;
}

Channel::~Channel() {
}

void Channel::set_topic(std::string new_topic) {
	topic = new_topic;
	topic_set_time = time(0);
}

std::string Channel::get_topic() const {
	return topic;
}

void Channel::set_mode(std::string new_mode) {
	mode = new_mode;
}

std::string Channel::get_mode() const {
	// if mode string is empty, return empty string
	if (mode.empty()) {
		return "";
	}

	// Create a new string for ordered modes
	std::string ordered_mode = "+";
	
	// Add modes in desired order
	if (channel_mode.k == 1) ordered_mode += "k";
	if (channel_mode.l == 1) ordered_mode += "l";
	if (channel_mode.i == 1) ordered_mode += "i";
	if (channel_mode.t == 1) ordered_mode += "t";
	if (channel_mode.o == 1) ordered_mode += "o";
	
	return ordered_mode;
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

std::string Channel::get_key() const {
	return key;
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

void Channel::setKey(std::string new_key) { key = new_key; }
void Channel::removeKey() { key = ""; }
bool Channel::hasKey() const { return key != ""; }

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

const Client &Channel::getCreator() const {
	return creator;
}

void Channel::setCreator(const Client &client) {
	creator = client;
	// Set the creator as an operator
	addOperator(client);
	// Set the operator mode flag
	channel_mode.o = 1;
	// Set initial mode string with +o
	mode = "+o";
}

// mode management methods
void Channel::setMode(char mode, int value) {
	switch (mode) {
		case 'i':
			channel_mode.i = value;
			invite_only = (value == 1);
			break;
		case 't':
			channel_mode.t = value;
			topic_restricted = (value == 1);
			break;
		case 'k':
			channel_mode.k = value;
			break;
		case 'o':
			channel_mode.o = value;
			break;
		case 'l':
			channel_mode.l = value;
			break;
	}
	
	// Initialize mode string if empty
	if (this->mode.empty()) {
		this->mode = "+";
	}
	
	// Add or remove mode while maintaining order
	if (value == 1) {
		// Only add if not already present
		if (this->mode.find(mode) == std::string::npos) {
			// If this is the first mode, just add it
			if (this->mode == "+") {
				this->mode += mode;
			} else {
				// Otherwise, insert it at the end
				this->mode.insert(this->mode.length(), 1, mode);
			}
		}
	} else {
		// Remove the mode if it exists
		size_t pos = this->mode.find(mode);
		if (pos != std::string::npos) {
			this->mode.erase(pos, 1);
		}
		// If no modes left, clear the string
		if (this->mode == "+") {
			this->mode.clear();
		}
	}
}

int Channel::getMode(char mode) const {
	switch (mode) {
		case 'i': return channel_mode.i;
		case 't': return channel_mode.t;
		case 'k': return channel_mode.k;
		case 'o': return channel_mode.o;
		case 'l': return channel_mode.l;
		default: return -1;
	}
}

void Channel::updateModeString() {
	// Ensure mode string starts with +
	if (!mode.empty() && mode[0] != '+') {
		mode = "+" + mode;
	}
	
	// If we have a creator and no operator mode, add it
	if (creator.getNickname() != "" && mode.find("o") == std::string::npos) {
		mode += "o";
	}
}

const MODE& Channel::getChannelMode() const {
	return channel_mode;
}

const std::vector<Client>& Channel::get_invited_users() const {
	return invited_users;
}
int Channel::get_max_clients() const {
	return max_client;
}
void Channel::add_invited_user(const Client &client) {
	invited_users.push_back(client);
}
void Channel::remove_invited_user(const Client &client){
    for (size_t i = 0; i < invited_users.size(); i++) {
        if (invited_users[i] == client) {
            invited_users.erase(invited_users.begin() + i);
            break;
        }
    }
}

void Channel::set_topic_time(time_t t) {
	topic_set_time = t;
}

time_t Channel::get_topic_time() const {
	return topic_set_time;
}