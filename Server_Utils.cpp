#include "Server.hpp"

std::string normalizeChannelName(const std::string& channel_name) {
    std::string normalized = channel_name;
    if (!normalized.empty() && normalized[0] == '&') {
        normalized[0] = '#';
    }
    return normalized;
}

void Server::handleJoin(size_t i, int client_fd, const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        std::string error_msg = ":irc 461 " + clients[i - 1].getNickname() + " JOIN :Not enough parameters\r\n";
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    // parse channel name and key
    std::string channel_name = tokens[1];
    if (!channel_name.empty() && channel_name[channel_name.size() - 1] == '\n') {
        channel_name.erase(channel_name.size() - 1);
    }        
    std::string key = "";
    if (tokens.size() >= 3)
        key = tokens[2];

    // validate channel name
    if (channel_name[0] != '#' && channel_name[0] != '&') {
        std::string error_msg = ":irc 403 " + clients[i - 1].getNickname() + " " + channel_name + " :No such channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    // Normalize channel name
    std::string normalized_channel = normalizeChannelName(channel_name);

    // check if channel exists
    bool channel_exists = false;
    Channel *target_channel = NULL;
    
    for (size_t j = 0; j < channels.size(); j++) {
        if (channels[j].get_name() == normalized_channel) {
            channel_exists = true;
            target_channel = &channels[j];
            break;
        }
    }

    if (!channel_exists) {
        // create new channel
        int channel_key = -1;
        if (!key.empty()) {
            std::istringstream iss(key);
            iss >> channel_key;
        }
        Channel new_channel(normalized_channel, channel_key);
        // add the creator as operator and set them as creator
        new_channel.addOperator(clients[i - 1]);
        new_channel.setCreator(clients[i - 1]);
        channels.push_back(new_channel);
        target_channel = &channels.back();
    }

    if (target_channel) {
        // check if channel need a key
        int channel_key = target_channel->get_key();
        int provided_key = -1;
        if (!key.empty()) {
            std::istringstream iss(key);
            iss >> provided_key;
        }
        
        if (channel_key != -1 && (key.empty() || channel_key != provided_key)) {
            std::string error_msg = ":irc 475 " + clients[i - 1].getNickname() + " " + normalized_channel + " :Cannot join channel (+k)\r\n";
            send(client_fd, error_msg.c_str(), error_msg.length(), 0);
            return;
        }

        // check if channel is invite-only and user is not invited
        if (target_channel->getMode('i') == 1) {
            bool is_invited = false;
            const std::vector<Client> &invited_users = target_channel->get_invited_users();
            for (size_t j = 0; j < invited_users.size(); j++) {
                if (invited_users[j].getFd() == clients[i - 1].getFd()) {
                    is_invited = true;
                    break;
                }
            }
            if (!is_invited) {
                std::string error_msg = ":irc 473 " + clients[i - 1].getNickname() + " " + normalized_channel + " :Cannot join channel (+i)\r\n";
                send(client_fd, error_msg.c_str(), error_msg.length(), 0);
                return;
            }
            // remove user from invited list after successful join
            target_channel->remove_invited_user(clients[i - 1]);
        }

        // check if channel has user limit and is full
        if (target_channel->getMode('l') == 1) {
            int max_clients = target_channel->get_max_clients();
            const std::vector<Client> &channel_clients = target_channel->get_clients();
            if (max_clients > 0 && static_cast<int>(channel_clients.size()) >= max_clients) {
                std::string error_msg = ":irc 471 " + clients[i - 1].getNickname() + " " + normalized_channel + " :Cannot join channel (+l)\r\n";
                send(client_fd, error_msg.c_str(), error_msg.length(), 0);
                return;
            }
        }

        // check if user is already in the channel
        const std::vector<Client> &channel_clients = target_channel->get_clients();
        for (size_t j = 0; j < channel_clients.size(); j++) {
            if (channel_clients[j].getFd() == clients[i - 1].getFd()) {
                // user is already in the channel, just send them the topic if it exists
                if (!target_channel->get_topic().empty()) {
                    std::string topic_msg = ":irc 332 " + clients[i - 1].getNickname() + " " + normalized_channel + " :" + target_channel->get_topic() + "\r\n";
                    send(client_fd, topic_msg.c_str(), topic_msg.length(), 0);
                }
                return;
            }
        }

        // add client to channel
        target_channel->add_client(clients[i - 1]);
        
        // send JOIN message to all clients in channel including the joiner
        std::string join_msg = ":" + clients[i - 1].getNickname() + "!~" + clients[i - 1].getUsername() + "@localhost JOIN " + normalized_channel + "\r\n";
        for (size_t j = 0; j < channel_clients.size(); j++) {
            send(channel_clients[j].getFd(), join_msg.c_str(), join_msg.length(), 0);
        }
        
        // send RPL_TOPIC if exists
        if (!target_channel->get_topic().empty()) {
            std::string topic_msg = ":irc 332 " + clients[i - 1].getNickname() + " " + normalized_channel + " :" + target_channel->get_topic() + "\r\n";
            send(client_fd, topic_msg.c_str(), topic_msg.length(), 0);
        }

        // send RPL_NAMREPLY
        std::string names_msg = ":irc 353 " + clients[i - 1].getNickname() + " = " + normalized_channel + " :";
        const std::vector<Client> &clients_list = target_channel->get_clients();
        for (size_t j = 0; j < clients_list.size(); j++) {
            if (j != 0)
                names_msg += " ";
            // Add @ prefix for operators and creator
            if (target_channel->isOperator(clients_list[j]) || target_channel->getCreator().getFd() == clients_list[j].getFd())
                names_msg += "@";
            names_msg += clients_list[j].getNickname();
        }
        names_msg += "\r\n";
        send(client_fd, names_msg.c_str(), names_msg.length(), 0);

        // send RPL_ENDOFNAMES
        std::string end_names_msg = ":irc 366 " + clients[i - 1].getNickname() + " " + normalized_channel + " :End of /NAMES list\r\n";
        send(client_fd, end_names_msg.c_str(), end_names_msg.length(), 0);
    }
}

void Server::handlePrivmsg(size_t i, int client_fd, const std::vector<std::string>& tokens) {
    if (tokens.size() < 3) {
        std::string error_msg = ":irc 461 " + clients[i - 1].getNickname() + " PRIVMSG :Not enough parameters\r\n";
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    std::string target = tokens[1];
    std::string message;

    // combine all remaining tokens into the message
    for (size_t j = 2; j < tokens.size(); j++) {
        if (j > 2)
            message += " ";
        message += tokens[j];
    }

    // Remove trailing newline and carriage return if present
    if (!message.empty()) {
        message.erase(message.find_last_not_of("\r\n") + 1);
    }

    // Remove leading colon if present (standard IRC format)
    if (!message.empty() && message[0] == ':') {
        message = message.substr(1);
    }

    // Check if message is empty after processing
    if (message.empty()) {
        std::string error_msg = ":irc 412 " + clients[i - 1].getNickname() + " :No text to send\r\n";
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    // check if target is a channel
    if (target[0] == '#' || target[0] == '&') {
        // Normalize channel name
        std::string normalized_target = normalizeChannelName(target);
        
        // find the channel
        Channel *target_channel = NULL;
        for (size_t j = 0; j < channels.size(); j++) {
            if (channels[j].get_name() == normalized_target) {
                target_channel = &channels[j];
                break;
            }
        }

        if (!target_channel) {
            std::string error_msg = ":irc 403 " + clients[i - 1].getNickname() + " " + target + " :No such channel\r\n";
            send(client_fd, error_msg.c_str(), error_msg.length(), 0);
            return;
        }

        // check if sender is in the channel
        bool is_in_channel = false;
        const std::vector<Client> &channel_clients = target_channel->get_clients();
        for (size_t j = 0; j < channel_clients.size(); j++) {
            if (channel_clients[j].getFd() == clients[i - 1].getFd()) {
                is_in_channel = true;
                break;
            }
        }

        if (!is_in_channel) {
            std::string error_msg = ":irc 404 " + clients[i - 1].getNickname() + " " + target + " :Cannot send to channel\r\n";
            send(client_fd, error_msg.c_str(), error_msg.length(), 0);
            return;
        }

        // send message to all clients in channel
        std::string privmsg;
        // Add @ prefix if sender is operator or creator
        if (target_channel->isOperator(clients[i - 1]) || target_channel->getCreator().getFd() == clients[i - 1].getFd())
            privmsg = ":" + std::string("@") + clients[i - 1].getNickname() + "!~" + clients[i - 1].getUsername() + "@localhost PRIVMSG " + target + " :";
        else
            privmsg = ":" + clients[i - 1].getNickname() + "!~" + clients[i - 1].getUsername() + "@localhost PRIVMSG " + target + " :";
        privmsg += message + "\r\n";
        for (size_t j = 0; j < channel_clients.size(); j++) {
            if (channel_clients[j].getFd() != clients[i - 1].getFd()) { // Don't send to self
                send(channel_clients[j].getFd(), privmsg.c_str(), privmsg.length(), 0);
            }
        }
    }
    else {
        // target is a user
        bool user_found = false;
        for (size_t j = 0; j < clients.size(); j++) {
            if (clients[j].getNickname() == target) {
                user_found = true;
                std::string privmsg = ":" + clients[i - 1].getNickname() + "!~" + clients[i - 1].getUsername() + "@localhost PRIVMSG " + target + " :" + message + "\r\n";
                send(clients[j].getFd(), privmsg.c_str(), privmsg.length(), 0);
                break;
            }
        }

        if (!user_found) {
            std::string error_msg = ":irc 401 " + clients[i - 1].getNickname() + " " + target + " :No such nick\r\n";
            send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        }
    }
}

void Server::handleKick(size_t i, int client_fd, const std::vector<std::string>& tokens) {
    if (tokens.size() < 3) {
        std::string error_msg = ":irc 461 " + clients[i - 1].getNickname() + " KICK :Not enough parameters\r\n";
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    std::string channel_name = tokens[1];
    std::string target_nick = tokens[2];
    if (!channel_name.empty() && channel_name[channel_name.size() - 1] == '\n') {
        channel_name.erase(channel_name.size() - 1);
    }
    if (!target_nick.empty() && target_nick[target_nick.size() - 1] == '\n') {
        target_nick.erase(target_nick.size() - 1);
    }

    // validate channel name
    if (channel_name[0] != '#' && channel_name[0] != '&') {
        std::string error_msg = ":irc 403 " + clients[i - 1].getNickname() + " " + channel_name + " :No such channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    // Normalize channel name
    std::string normalized_channel = normalizeChannelName(channel_name);

    // find the channel
    Channel *target_channel = NULL;
    for (size_t j = 0; j < channels.size(); j++) {
        if (channels[j].get_name() == normalized_channel) {
            target_channel = &channels[j];
            break;
        }
    }

    if (!target_channel) {
        std::string error_msg = ":irc 403 " + clients[i - 1].getNickname() + " " + normalized_channel + " :No such channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    // check if kicker is in the channel
    bool kicker_in_channel = false;
    const std::vector<Client> &channel_clients = target_channel->get_clients();
    for (size_t j = 0; j < channel_clients.size(); j++) {
        if (channel_clients[j] == clients[i - 1]) {
            kicker_in_channel = true;
            break;
        }
    }

    if (!kicker_in_channel) {
        std::string error_msg = ":irc 442 " + clients[i - 1].getNickname() + " " + normalized_channel + " :You're not on that channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    // find target client in channel
    bool target_found = false;
    Client *target_client = NULL;
    for (size_t j = 0; j < channel_clients.size(); j++) {
        if (channel_clients[j].getNickname() == target_nick) {
            target_found = true;
            target_client = const_cast<Client*>(&channel_clients[j]);
            break;
        }
    }

    if (!target_found) {
        std::string error_msg = ":irc 441 " + clients[i - 1].getNickname() + " " + target_nick + " " + normalized_channel + " :They aren't on that channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    // check if kicker is creator or operator
    if (target_channel->getCreator() != clients[i - 1] && !target_channel->isOperator(clients[i - 1])) {
        std::string error_msg = ":irc 482 " + clients[i - 1].getNickname() + " " + normalized_channel + " :You're not channel operator\r\n";
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    // remove client from channel
    target_channel->delete_client(*target_client);

    // send KICK message to all clients in channel
    std::string kick_msg;
    // Add @ prefix if kicker is operator or creator
    if (target_channel->isOperator(clients[i - 1]) || target_channel->getCreator() == clients[i - 1])
        kick_msg = ":" + std::string("@") + clients[i - 1].getNickname() + "!~" + clients[i - 1].getUsername() + "@localhost KICK " + normalized_channel + " " + target_nick;
    else
        kick_msg = ":" + clients[i - 1].getNickname() + "!~" + clients[i - 1].getUsername() + "@localhost KICK " + normalized_channel + " " + target_nick;
    
    // Add reason if provided
    if (tokens.size() > 3) {
        std::string reason;
        for (size_t j = 3; j < tokens.size(); j++) {
            if (j > 3) reason += " ";
            if (j == 3 && !tokens[j].empty() && tokens[j][0] == ':') {
                reason += tokens[j].substr(1);
            } else {
                reason += tokens[j];
            }
        }
        kick_msg += " :" + reason;
    }
    kick_msg += "\r\n";
    
    for (size_t j = 0; j < channel_clients.size(); j++) {
        send(channel_clients[j].getFd(), kick_msg.c_str(), kick_msg.length(), 0);
    }
}

void Server::handleInvite(size_t i, int client_fd, const std::vector<std::string>& tokens) {
    if (tokens.size() < 3) {
        std::string error_msg = ":irc 461 " + clients[i - 1].getNickname() + " INVITE :Not enough parameters\r\n";
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    std::string target_nick = tokens[1];
    std::string channel_name = tokens[2];
    if (!channel_name.empty() && channel_name[channel_name.size() - 1] == '\n') {
        channel_name.erase(channel_name.size() - 1);
    }
    if (!target_nick.empty() && target_nick[target_nick.size() - 1] == '\n') {
        target_nick.erase(target_nick.size() - 1);
    }

    // validate channel name
    if (channel_name[0] != '#' && channel_name[0] != '&') {
        std::string error_msg = ":irc 403 " + clients[i - 1].getNickname() + " " + channel_name + " :No such channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    // Normalize channel name
    std::string normalized_channel = normalizeChannelName(channel_name);

    // find the channel
    Channel *target_channel = NULL;
    for (size_t j = 0; j < channels.size(); j++) {
        if (channels[j].get_name() == normalized_channel) {
            target_channel = &channels[j];
            break;
        }
    }

    if (!target_channel) {
        std::string error_msg = ":irc 403 " + clients[i - 1].getNickname() + " " + normalized_channel + " :No such channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    // check if inviter is in the channel
    bool inviter_in_channel = false;
    const std::vector<Client> &channel_clients = target_channel->get_clients();
    for (size_t j = 0; j < channel_clients.size(); j++) {
        if (channel_clients[j] == clients[i - 1]) {
            inviter_in_channel = true;
            break;
        }
    }

    if (!inviter_in_channel) {
        std::string error_msg = ":irc 442 " + clients[i - 1].getNickname() + " " + normalized_channel + " :You're not on that channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    // find target client in global client list
    bool target_found = false;
    Client *target_client = NULL;
    for (size_t j = 0; j < clients.size(); j++) {
        if (clients[j].getNickname() == target_nick) {
            target_found = true;
            target_client = &clients[j];
            break;
        }
    }

    if (!target_found) {
        std::string error_msg = ":irc 401 " + clients[i - 1].getNickname() + " " + target_nick + " :No such nick\r\n";
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    // check if target is already in the channel
    for (size_t j = 0; j < channel_clients.size(); j++) {
        if (channel_clients[j] == *target_client) {
            std::string error_msg = ":irc 443 " + clients[i - 1].getNickname() + " " + target_nick + " " + normalized_channel + " :is already on channel\r\n";
            send(client_fd, error_msg.c_str(), error_msg.length(), 0);
            return;
        }
    }

    // send INVITE message to target client
    std::string invite_msg = ":" + clients[i - 1].getNickname() + "!~" + clients[i - 1].getUsername() + "@localhost INVITE " + target_nick + " " + normalized_channel + "\r\n";
    send(target_client->getFd(), invite_msg.c_str(), invite_msg.length(), 0);

    // add user to invited list
    target_channel->add_invited_user(*target_client);

    // send RPL_INVITING to inviter
    std::string inviting_msg = ":irc 341 " + clients[i - 1].getNickname() + " " + target_nick + " " + normalized_channel + "\r\n";
    send(client_fd, inviting_msg.c_str(), inviting_msg.length(), 0);
}

void Server::handleTopic(size_t i, int client_fd, const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        std::string error_msg = ":irc 461 " + clients[i - 1].getNickname() + " TOPIC :Not enough parameters\r\n";
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    std::string channel_name = tokens[1];
    // Normalize channel name
    std::string normalized_channel = normalizeChannelName(channel_name);
    Channel *target_channel = NULL;

    // find the channel
    for (size_t j = 0; j < channels.size(); j++) {
        if (channels[j].get_name() == normalized_channel) {
            target_channel = &channels[j];
            break;
        }
    }

    if (!target_channel) {
        std::string error_msg = ":irc 403 " + clients[i - 1].getNickname() + " " + normalized_channel + " :No such channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    // check if user is in the channel
    bool user_in_channel = false;
    const std::vector<Client> &channel_clients = target_channel->get_clients();
    for (size_t j = 0; j < channel_clients.size(); j++) {
        if (channel_clients[j].getFd() == clients[i - 1].getFd()) {
            user_in_channel = true;
            break;
        }
    }

    if (!user_in_channel) {
        std::string error_msg = ":irc 442 " + clients[i - 1].getNickname() + " " + normalized_channel + " :You're not on that channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    // if no topic provided, show current topic (allowed for all users)
    if (tokens.size() == 2) {
        std::string topic = target_channel->get_topic();
        if (topic.empty()) {
            std::string msg = ":irc 331 " + clients[i - 1].getNickname() + " " + normalized_channel + " :No topic is set\r\n";
            send(client_fd, msg.c_str(), msg.length(), 0);
        }
        else {
            std::string msg = ":irc 332 " + clients[i - 1].getNickname() + " " + normalized_channel + " :" + topic + "\r\n";
            send(client_fd, msg.c_str(), msg.length(), 0);
        }
        return;
    }

    // if trying to change topic, check if user has permission
    if (target_channel->getMode('t') == 1 && 
        !target_channel->isOperator(clients[i - 1]) && 
        target_channel->getCreator().getFd() != clients[i - 1].getFd()) {
        std::string error_msg = ":irc 482 " + clients[i - 1].getNickname() + " " + normalized_channel + " :You're not channel operator\r\n";
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    // combine remaining tokens for topic
    std::string new_topic;
    for (size_t j = 2; j < tokens.size(); j++) {
        if (j > 2) new_topic += " ";
        if (j == 2 && !tokens[j].empty() && tokens[j][0] == ':') {
            new_topic += tokens[j].substr(1);
        } else {
            new_topic += tokens[j];
        }
    }

    // remove trailing newline if present
    if (!new_topic.empty() && new_topic[new_topic.size() - 1] == '\n') {
        new_topic.erase(new_topic.size() - 1);
    }

    // If topic is empty or just whitespace, clear the topic
    if (new_topic.empty() || new_topic.find_first_not_of(" \t\r\n") == std::string::npos) {
        new_topic = "";
    }

    target_channel->set_topic(new_topic);
    
    // notify all channel members of topic change
    std::string topic_msg;
    // Add @ prefix if topic setter is operator or creator
    if (target_channel->isOperator(clients[i - 1]) || target_channel->getCreator().getFd() == clients[i - 1].getFd())
        topic_msg = ":" + std::string("@") + clients[i - 1].getNickname() + "!~" + clients[i - 1].getUsername() + "@localhost TOPIC " + normalized_channel + " :" + new_topic;
    else
        topic_msg = ":" + clients[i - 1].getNickname() + "!~" + clients[i - 1].getUsername() + "@localhost TOPIC " + normalized_channel + " :" + new_topic;
    topic_msg += "\r\n";
    for (size_t j = 0; j < channel_clients.size(); j++) {
        send(channel_clients[j].getFd(), topic_msg.c_str(), topic_msg.length(), 0);
    }
}

void Server::handleMode(size_t i, int client_fd, const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        std::string error_msg = ":irc 461 " + clients[i - 1].getNickname() + " MODE :Not enough parameters\r\n";
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    std::string channel_name = tokens[1];
    if (channel_name[0] != '#' && channel_name[0] != '&') {
        std::string error_msg = ":irc 403 " + clients[i - 1].getNickname() + " " + channel_name + " :No such channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    // Normalize channel name
    std::string normalized_channel = normalizeChannelName(channel_name);

    // find the channel
    Channel *target_channel = NULL;
    for (size_t j = 0; j < channels.size(); j++) {
        if (channels[j].get_name() == normalized_channel) {
            target_channel = &channels[j];
            break;
        }
    }

    if (!target_channel) {
        std::string error_msg = ":irc 403 " + clients[i - 1].getNickname() + " " + normalized_channel + " :No such channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    // check if user is in the channel
    bool user_in_channel = false;
    const std::vector<Client> &channel_clients = target_channel->get_clients();
    for (size_t j = 0; j < channel_clients.size(); j++) {
        if (channel_clients[j].getFd() == clients[i - 1].getFd()) {
            user_in_channel = true;
            break;
        }
    }

    if (!user_in_channel) {
        std::string error_msg = ":irc 442 " + clients[i - 1].getNickname() + " " + normalized_channel + " :You're not on that channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    // if no mode change requested, show current modes
    if (tokens.size() == 2) {
        std::string mode_msg = ":irc 324 " + clients[i - 1].getNickname() + " " + normalized_channel + " " + target_channel->get_mode() + "\r\n";
        send(client_fd, mode_msg.c_str(), mode_msg.length(), 0);
        return;
    }

    // parse mode change
    std::string mode_str = tokens[2];
    if (mode_str.length() < 2 || (mode_str[0] != '+' && mode_str[0] != '-')) {
        std::string error_msg = ":irc 472 " + clients[i - 1].getNickname() + " " + mode_str + " :is unknown mode char to me\r\n";
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    char mode_char = mode_str[1];
    bool is_adding = (mode_str[0] == '+');
    int value = is_adding;

    // check if user has permission (creator or operator)
    bool has_permission = (target_channel->getCreator().getFd() == clients[i - 1].getFd()) || 
                        (target_channel->isOperator(clients[i - 1]));

    // special case for +o/-o which only creator can use
    if (mode_char == 'o' && target_channel->getCreator().getFd() != clients[i - 1].getFd()) {
        std::string error_msg = ":irc 482 " + clients[i - 1].getNickname() + " " + normalized_channel + " :You're not channel creator\r\n";
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    // for other modes, check if user has permission
    if (!has_permission) {
        std::string error_msg = ":irc 482 " + clients[i - 1].getNickname() + " " + normalized_channel + " :You're not channel operator\r\n";
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    // handle specific mode changes
    switch (mode_char) {
        case 'i':
            target_channel->setMode('i', value);
            break;
        case 't':
            target_channel->setMode('t', value);
            break;
        case 'k':
            if (is_adding) {
                if (tokens.size() < 4) {
                    std::string error_msg = ":irc 461 " + clients[i - 1].getNickname() + " MODE :Not enough parameters\r\n";
                    send(client_fd, error_msg.c_str(), error_msg.length(), 0);
                    return;
                }
                int new_key = atoi(tokens[3].c_str());
                target_channel->setKey(new_key);
            } else {
                target_channel->removeKey();
            }
            target_channel->setMode('k', value);
            break;
        case 'o':
            if (tokens.size() < 4) {
                std::string error_msg = ":irc 461 " + clients[i - 1].getNickname() + " MODE :Not enough parameters\r\n";
                send(client_fd, error_msg.c_str(), error_msg.length(), 0);
                return;
            }
            {
                std::string target_nick = tokens[3];
                // find target client
                bool target_found = false;
                for (size_t j = 0; j < channel_clients.size(); j++) {
                    if (channel_clients[j].getNickname() == target_nick) {
                        target_found = true;
                        if (is_adding) {
                            target_channel->addOperator(channel_clients[j]);
                        } else {
                            target_channel->removeOperator(channel_clients[j]);
                        }
                        break;
                    }
                }
                if (!target_found) {
                    std::string error_msg = ":irc 441 " + clients[i - 1].getNickname() + " " + target_nick + " " + normalized_channel + " :They aren't on that channel\r\n";
                    send(client_fd, error_msg.c_str(), error_msg.length(), 0);
                    return;
                }
            }
            target_channel->setMode('o', value);
            break;
        case 'l':
            if (is_adding) {
                if (tokens.size() < 4) {
                    std::string error_msg = ":irc 461 " + clients[i - 1].getNickname() + " MODE :Not enough parameters\r\n";
                    send(client_fd, error_msg.c_str(), error_msg.length(), 0);
                    return;
                }
                int limit = atoi(tokens[3].c_str());
                if (limit <= 0) {
                    std::string error_msg = ":irc 461 " + clients[i - 1].getNickname() + " MODE :Invalid limit value\r\n";
                    send(client_fd, error_msg.c_str(), error_msg.length(), 0);
                    return;
                }
                target_channel->setMaxClients(limit);
            } else {
                target_channel->removeUserLimit();
            }
            target_channel->setMode('l', value);
            break;
        default:
            std::string error_msg = ":irc 472 " + clients[i - 1].getNickname() + " " + std::string(1, mode_char) + " :is unknown mode char to me\r\n";
            send(client_fd, error_msg.c_str(), error_msg.length(), 0);
            return;
    }

    // notify all channel members of mode change
    std::string mode_change_msg;
    // Add @ prefix if mode setter is operator or creator
    if (target_channel->isOperator(clients[i - 1]) || target_channel->getCreator().getFd() == clients[i - 1].getFd())
        mode_change_msg = ":" + std::string("@") + clients[i - 1].getNickname() + "!~" + clients[i - 1].getUsername() + "@localhost MODE " + normalized_channel + " " + mode_str;
    else
        mode_change_msg = ":" + clients[i - 1].getNickname() + "!~" + clients[i - 1].getUsername() + "@localhost MODE " + normalized_channel + " " + mode_str;
    if (tokens.size() > 3) {
        mode_change_msg += " " + tokens[3];
    }
    mode_change_msg += "\r\n";
    for (size_t j = 0; j < channel_clients.size(); j++) {
        send(channel_clients[j].getFd(), mode_change_msg.c_str(), mode_change_msg.length(), 0);
    }
}
