#include "../Server.hpp"

void Server::handleTopic(size_t i, int client_fd, const std::vector<std::string> &tokens) {
    Client &client = clients[i - 1];
    const std::string &nickname = client.getNickname();

    if (tokens.size() < 2 || tokens[1].empty()) {
        sendError(client_fd, 461, nickname, "", "Not enough parameters.");
        return;
    }

    const std::string &channel_name = tokens[1];
    Channel *target_channel = findChannelByName(channel_name);

    if (!target_channel) {
        sendError(client_fd, 403, nickname, channel_name, "No such channel");
        return;
    }

    if (!isUserInChannel(target_channel, client)) {
        sendError(client_fd, 442, nickname, channel_name, "You're not on that channel");
        return;
    }

    if (tokens.size() == 2) {
        sendCurrentTopic(client_fd, client, *target_channel, channel_name);
        return;
    }

    // only operator/creator can change topic if +t is set
    if (target_channel->isTopicRestricted() && !isUserOperatorOrCreator(target_channel, client)) {
        sendError(client_fd, 482, nickname, channel_name, "You're not a channel operator");
        return;
    }

    setNewTopicAndNotify(client, *target_channel, tokens[2], channel_name);
}

void Server::sendError(int client_fd, int code, const std::string &nickname, const std::string &channel, const std::string &message) {
    std::stringstream ss;
    ss << ": " << code << " " << nickname;
    if (!channel.empty())
        ss << " " << channel;
    ss << " :" << message << "\r\n";
    std::string error_msg = ss.str();
    send(client_fd, error_msg.c_str(), error_msg.length(), 0);
}

void Server::sendCurrentTopic(int client_fd, Client &client, Channel &channel, const std::string &channel_name) {
    const std::string &nickname = client.getNickname();
    const std::string &topic = channel.get_topic();

    if (topic.empty()) {
        std::string msg = ": 331 " + nickname + " " + channel_name + " :No topic is set\r\n";
        send(client_fd, msg.c_str(), msg.length(), 0);
    } else {
        std::string msg = ": 332 " + nickname + " " + channel_name + " :" + topic + "\r\n";
        send(client_fd, msg.c_str(), msg.length(), 0);

        std::stringstream ss;
        ss << channel.get_topic_time();
        std::string who_time_msg = ": 333 " + nickname + " " + channel_name + " " +
                                   channel.getCreator().getNickname() + " " + ss.str() + "\r\n";
        send(client_fd, who_time_msg.c_str(), who_time_msg.length(), 0);
    }
}

void Server::setNewTopicAndNotify(Client &client, Channel &channel, const std::string &new_topic, const std::string &channel_name) {
    channel.set_topic(new_topic);

    std::string topic_msg = ": 332 " + client.getNickname() + "!" + client.getUsername() + "@" + client.getIp() + " " +
                            channel_name + " :" + new_topic + "\r\n";

    const std::vector<Client> &channel_clients = channel.get_clients();
    for (size_t j = 0; j < channel_clients.size(); ++j) {
        send(channel_clients[j].getFd(), topic_msg.c_str(), topic_msg.length(), 0);
    }
}


