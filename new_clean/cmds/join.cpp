#include "../Server.hpp"

void Server::handleJoin(size_t i, int client_fd, const std::vector<std::string> &tokens) {
    Client &client = clients[i - 1];
    const std::string &nickname = client.getNickname();

    if (tokens.size() < 2 || tokens[1].empty()) {
        sendError(client_fd, 461, nickname, "", "Not enough parameters.");
        return;
    }

    std::string channel_name = tokens[1];
    std::string key = "";
    if (tokens.size() >= 3)
        key = tokens[2];


    if (!isValidChannelName(channel_name)) {
        sendError(client_fd, 403, nickname, channel_name, "No such channel");
        return;
    }

    Channel *target_channel = findOrCreateChannel(channel_name, key, client);

    if (!canClientJoinChannel(target_channel, client, channel_name, key, client_fd)) {
        return;
    }

    if (isClientAlreadyInChannel(target_channel, client, client_fd, channel_name)) {
        return;
    }

    target_channel->add_client(client);

    notifyJoin(target_channel, client, channel_name);
    sendTopicIfExists(target_channel, client_fd, nickname, channel_name);
    sendNamesList(target_channel, client_fd, nickname, channel_name);
}


bool Server::isValidChannelName(const std::string &name) {
    return (!name.empty() && (name[0] == '#' || name[0] == '&'));
}

Channel *Server::findOrCreateChannel(const std::string &name, const std::string &key, Client &creator) {
    for (size_t j = 0; j < channels.size(); ++j) {
        if (channels[j].get_name() == name) {
            return &channels[j];
        }
    }

    Channel new_channel(name, key);
    new_channel.addOperator(creator);
    new_channel.setCreator(creator);
    channels.push_back(new_channel);
    return &channels.back();
}

bool Server::canClientJoinChannel(Channel *channel, Client &client, const std::string &name, const std::string &key, int client_fd) {
    std::string channel_key = channel->get_key();
    std::string provided_key = key;

    if (channel->hasKey() && (provided_key.empty() || channel_key != provided_key)) {
        sendError(client_fd, 475, client.getNickname(), name, "Cannot join channel (+k)");
        return false;
    }

    if (channel->get_mode().find('i') != std::string::npos && !isClientInvited(channel, client)) {
        sendError(client_fd, 473, client.getNickname(), name, "Cannot join channel (+i)");
        return false;
    }

    if (channel->get_mode().find('l') != std::string::npos) {
        int max_clients = channel->get_max_clients();
        const std::vector<Client> &clients_list = channel->get_clients();
        if (max_clients > 0 && static_cast<int>(clients_list.size()) >= max_clients) {
            sendError(client_fd, 471, client.getNickname(), name, "Cannot join channel (+l)");
            return false;
        }
    }

    return true;
}

bool Server::isClientInvited(Channel *channel, Client &client) {
    const std::vector<Client> &invited = channel->get_invited_users();
    for (size_t j = 0; j < invited.size(); ++j) {
        if (invited[j] == client) {
            channel->remove_invited_user(client);
            return true;
        }
    }
    return false;
}

bool Server::isClientAlreadyInChannel(Channel *channel, Client &client, int client_fd, const std::string &name) {
    const std::vector<Client> &clients_list = channel->get_clients();
    for (size_t j = 0; j < clients_list.size(); ++j) {
        if (clients_list[j].getNickname() == client.getNickname()) {
            sendTopicIfExists(channel, client_fd, client.getNickname(), name);
            return true;
        }
    }
    return false;
}

void Server::notifyJoin(Channel *channel, Client &client, const std::string &name) {
    const std::vector<Client> &clients_list = channel->get_clients();
    // std::string join_msg = ":" + client.getNickname() + "!" + client.getUsername() + "@127.0.0.1 JOIN " + name + "\r\n";
    std::string join_msg = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getIp() + " JOIN " + name + "\r\n";

    for (size_t j = 0; j < clients_list.size(); ++j) {
        send(clients_list[j].getFd(), join_msg.c_str(), join_msg.length(), 0);
    }
}

void Server::sendTopicIfExists(Channel *channel, int client_fd, const std::string &nickname, const std::string &name) {
    if (!channel->get_topic().empty()) {
        std::string topic_msg = ": 332 " + nickname + " " + name + " :" + channel->get_topic() + "\r\n";
        send(client_fd, topic_msg.c_str(), topic_msg.length(), 0);
    }
}

void Server::sendNamesList(Channel *channel, int client_fd, const std::string &nickname, const std::string &name) {
    const std::vector<Client> &clients_list = channel->get_clients();
    std::string names_msg = ": 353 " + nickname + " @ " + name + " :";

    for (size_t j = 0; j < clients_list.size(); ++j) {
        if (j != 0)
            names_msg += " ";
        if (channel->isOperator(clients_list[j]) || channel->getCreator() == clients_list[j])
            names_msg += "@";
        names_msg += clients_list[j].getNickname();
    }
    names_msg += "\r\n";
    send(client_fd, names_msg.c_str(), names_msg.length(), 0);

    std::string end_names_msg = ": 366 " + nickname + " " + name + " :END of /NAMES list\r\n";
    send(client_fd, end_names_msg.c_str(), end_names_msg.length(), 0);
}
