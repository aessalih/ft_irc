#include "../Server.hpp"
#include <sstream>

void Server::handleMode(size_t i, int client_fd, const std::vector<std::string> &tokens) {
    if (tokens.size() < 2 || tokens[1].empty()) {
        sendError(client_fd, 461, clients[i - 1].getNickname(), "", "Not enough parameters.");
        return;
    }
    const std::string &channel_name = toLowerCase(tokens[1]);
    Channel *target_channel = findChannelByName(channel_name);
    if (!target_channel) {
        sendError(client_fd, 403, clients[i - 1].getNickname(), channel_name, "No such channel");
        return;
    }
    // show current modes if no mode string is provided
    if (tokens.size() == 2) {
        std::string mode_str = target_channel->get_mode();
        std::string msg = ": 324 " + clients[i - 1].getNickname() + " " + channel_name + " " + mode_str;
        std::stringstream ss;
        // if +k is set, append the key value
        if (target_channel->hasKey()) {
            ss << " " << target_channel->get_key();
        }
        // if +l is set, append the limit value
        if (target_channel->hasUserLimit()) {
            ss << " " << target_channel->get_max_clients();
        }
        msg += ss.str();
        // append operator nicks
        const std::vector<Client> &chan_clients = target_channel->get_clients();
        for (size_t opi = 0; opi < chan_clients.size(); ++opi) {
            if (target_channel->isOperator(chan_clients[opi]) || target_channel->getCreator() == chan_clients[opi]) {
                msg += " @" + chan_clients[opi].getNickname();
            }
        }
        msg += "\r\n";
        send(client_fd, msg.c_str(), msg.length(), 0);
        return;
    }
    // only operator/creator can change modes
    if (!isUserOperatorOrCreator(target_channel, clients[i - 1])) {
        sendError(client_fd, 482, clients[i - 1].getNickname(), channel_name, "You're not a channel operator");
        return;
    }
    std::string modes = tokens[2];
    bool adding = true;
    size_t param_idx = 3;
    for (size_t j = 0; j < modes.size(); ++j) {
        char c = modes[j];
        if (c == '+') {
            adding = true;
        } else if (c == '-') {
            adding = false;
        } else if (c == 'i') {
            handleModeInvite(target_channel, adding, client_fd, clients[i - 1].getNickname(), channel_name);}
        else if (c == 'l') {
            if (adding) {
                if (param_idx >= tokens.size() || tokens[param_idx].empty()) {
                    std::string err = ": 461 " + clients[i - 1].getNickname() + " :Not enough parameters.\r\n";
                    send(client_fd, err.c_str(), err.length(), 0);
                    return;
                }
                handleModeLimit(target_channel, adding, tokens[param_idx++], client_fd, clients[i - 1].getNickname(), channel_name);
            } else {
                handleModeLimit(target_channel, adding, "", client_fd, clients[i - 1].getNickname(), channel_name);
            }
        } else if (c == 't') {
            handleModeTopic(target_channel, adding, client_fd, clients[i - 1].getNickname(), channel_name);}
        else if (c == 'o') {
            if (param_idx >= tokens.size()) {
                sendError(client_fd, 461, clients[i - 1].getNickname(), channel_name, "Not enough parameters.");
                return;
            }
            handleModeOperator(target_channel, adding, tokens[param_idx++], client_fd, clients[i - 1].getNickname(), channel_name);}
        else if (c == 'k') {
            if (adding) {
                if (param_idx >= tokens.size()) {
                    sendError(client_fd, 461, clients[i - 1].getNickname(), channel_name, "Not enough parameters.");
                    return;
                }
                handleModeKey(target_channel, adding, tokens[param_idx++], client_fd, clients[i - 1].getNickname(), channel_name);
            } else {
                handleModeKey(target_channel, adding, "", client_fd, clients[i - 1].getNickname(), channel_name);
            }
        } else {
            // custom error message for unknown mode char
            std::string err_target;
            if (modes[0] == '+' || modes[0] == '-') {
                err_target = channel_name + " " + modes;
            } else {
                err_target = channel_name + " " + modes;
            }
            std::string err_msg = ": 472 " + clients[i - 1].getNickname() + " " + err_target + " :is not a recognised channel mode\r\n";
            send(client_fd, err_msg.c_str(), err_msg.length(), 0);
            return;
        }
    }
}

void Server::handleModeInvite(Channel *channel, bool adding, int client_fd, const std::string &nickname, const std::string &channel_name) {
    // +i: set invite-only, -i: remove invite-only
    (void) client_fd;
    if (adding) {
            channel->setInviteOnly(true);
            channel->setMode('i', 1);
        }
     else {
            channel->setInviteOnly(false);
            channel->setMode('i', 0);
        // 
    }
    // broadcast mode change 
    const std::vector<Client> &chan_clients = channel->get_clients();
    std::string mode_msg;
    // find the user for the nickname
    const Client *sender = NULL;
    for (size_t k = 0; k < chan_clients.size(); ++k) {
        if (chan_clients[k].getNickname() == nickname) {
            sender = &chan_clients[k];
            break;
        }
    }
    std::string username = (sender ? sender->getUsername() : "unknown");
    mode_msg = ":" + nickname + "!" + username + " MODE " + channel_name + " " + (adding ? "+i" : "-i") + "\r\n";
    for (size_t j = 0; j < chan_clients.size(); ++j) {
        send(chan_clients[j].getFd(), mode_msg.c_str(), mode_msg.length(), 0);
    }
}

void Server::handleModeLimit(Channel *channel, bool adding, const std::string &param, int client_fd, const std::string &nickname, const std::string &channel_name) {
    const std::vector<Client> &chan_clients = channel->get_clients();
    const Client *sender = NULL;
    for (size_t k = 0; k < chan_clients.size(); ++k) {
        if (chan_clients[k].getNickname() == nickname) {
            sender = &chan_clients[k];
            break;
        }
    }
    std::string username = (sender ? sender->getUsername() : "unknown");
    if (adding) {
        if (param.empty()) {
            std::string err = ": 461 " + nickname + " :Not enough parameters.\r\n";
            send(client_fd, err.c_str(), err.length(), 0);
            return;
        }
        // +l: set user limit
        int limit = 0;
        for (size_t i = 0; i < param.size(); ++i) {
            if (param[i] < '0' || param[i] > '9') {
                std::string err = ": 461 " + nickname + " :Not enough parameters.\r\n";
                send(client_fd, err.c_str(), err.length(), 0);
                return;
            }
            limit = limit * 10 + (param[i] - '0');
        }
        if (limit <= 0) {
            std::string err = ": 461 " + nickname + " :Not enough parameters.\r\n";
            send(client_fd, err.c_str(), err.length(), 0);
            return;
        }
        // check if limit is less than current users
        if ((int)chan_clients.size() > limit) {
            std::string err = ": 696 " + channel_name + " Invalid mode parameter l\r\n";
            send(client_fd, err.c_str(), err.length(), 0);
            return;
        }

        channel->setMaxClients(limit);
        channel->setMode('l', 1);
        std::string mode_msg = ":" + nickname + "!" + username + " MODE " + channel_name + " +l " + param + "\r\n";
        for (size_t j = 0; j < chan_clients.size(); ++j) {
            send(chan_clients[j].getFd(), mode_msg.c_str(), mode_msg.length(), 0);
        }
    } else {
        channel->removeUserLimit();
        channel->setMode('l', 0);
        std::string mode_msg = ":" + nickname + "!" + username + " MODE " + channel_name + " -l\r\n";
        for (size_t j = 0; j < chan_clients.size(); ++j) {
            send(chan_clients[j].getFd(), mode_msg.c_str(), mode_msg.length(), 0);
        }
    }
}

void Server::handleModeTopic(Channel *channel, bool adding, int client_fd, const std::string &nickname, const std::string &channel_name) {
    // +t: set topic restriction, -t: remove topic restriction
    (void) client_fd;
    if (adding) {
        
            channel->setTopicRestricted(true);
            channel->setMode('t', 1);
    } else {
            channel->setTopicRestricted(false);
            channel->setMode('t', 0);
    }
    // broadcast mode change to all channel members
    const std::vector<Client> &chan_clients = channel->get_clients();
    const Client *sender = NULL;
    for (size_t k = 0; k < chan_clients.size(); ++k) {
        if (chan_clients[k].getNickname() == nickname) {
            sender = &chan_clients[k];
            break;
        }
    }
    std::string username = (sender ? sender->getUsername() : "unknown");
    std::string mode_msg = ":" + nickname + "!" + username + " MODE " + channel_name + " " + (adding ? "+t" : "-t") + "\r\n";
    for (size_t j = 0; j < chan_clients.size(); ++j) {
        send(chan_clients[j].getFd(), mode_msg.c_str(), mode_msg.length(), 0);
    }
}

void Server::handleModeKey(Channel *channel, bool adding, const std::string &param, int client_fd, const std::string &nickname, const std::string &channel_name) {
    const std::vector<Client> &chan_clients = channel->get_clients();
    const Client *sender = NULL;
    for (size_t k = 0; k < chan_clients.size(); ++k) {
        if (chan_clients[k].getNickname() == nickname) {
            sender = &chan_clients[k];
            break;
        }
    }
    std::string username = (sender ? sender->getUsername() : "unknown");
    if (adding) {
        if (param.empty()) {
            std::string err = ": 461 " + nickname + " :Not enough parameters.\r\n";
            send(client_fd, err.c_str(), err.length(), 0);
            return;
        }
        if (channel->hasKey() && channel->get_key() == param) {
            return; // already set to this key
        }
        channel->setKey(param);
        channel->setMode('k', 1);
        std::string mode_msg = ":" + nickname + "!" + username + " MODE " + channel_name + " +k " + param + "\r\n";
        for (size_t j = 0; j < chan_clients.size(); ++j) {
            send(chan_clients[j].getFd(), mode_msg.c_str(), mode_msg.length(), 0);
        }
    } else {
        if (!channel->hasKey()) {
            return; // already unset
        }
        channel->removeKey();
        channel->setMode('k', 0);
        std::string mode_msg = ":" + nickname + "!" + username + " MODE " + channel_name + " -k\r\n";
        for (size_t j = 0; j < chan_clients.size(); ++j) {
            send(chan_clients[j].getFd(), mode_msg.c_str(), mode_msg.length(), 0);
        }
    }
}

void Server::handleModeOperator(Channel *channel, bool adding, const std::string &param, int client_fd, const std::string &nickname, const std::string &channel_name) {
    // +o: add operator, -o: remove operator
    const std::vector<Client> &chan_clients = channel->get_clients();
    const Client *sender = NULL;
    for (size_t k = 0; k < chan_clients.size(); ++k) {
        if (chan_clients[k].getNickname() == nickname) {
            sender = &chan_clients[k];
            break;
        }
    }
    std::string username = (sender ? sender->getUsername() : "unknown");
    // find the target user in the channel
    Client *target = NULL;
    for (size_t j = 0; j < chan_clients.size(); ++j) {
        if (chan_clients[j].getNickname() == param) {
            target = const_cast<Client*>(&chan_clients[j]);
            break;
        }
    }
    if (!target) {
        std::string err = ": 441 " + nickname + " " + param + " :They aren't on that channel\r\n";
        send(client_fd, err.c_str(), err.length(), 0);
        return;
    }
    // don't allow changing op status for the creator
    if (channel->getCreator() == *target) {
        return;
    }
    if (adding)
        channel->addOperator(*target);
    else
        channel->removeOperator(*target);

    // broadcast mode change to all channel members
    std::string mode_msg = ":" + nickname + "!" + username + " MODE " + channel_name + " " + (adding ? "+o " : "-o ") + param + "\r\n";
    for (size_t j = 0; j < chan_clients.size(); ++j) {
        send(chan_clients[j].getFd(), mode_msg.c_str(), mode_msg.length(), 0);
    }
}

