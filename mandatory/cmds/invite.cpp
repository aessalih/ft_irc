#include "../Server.hpp"
#include <sstream>

// Helper function to split a string by comma and return a vector of trimmed strings
static std::vector<std::string> splitByComma(const std::string &input) {
    std::vector<std::string> result;
    std::stringstream ss(input);
    std::string item;
    while (std::getline(ss, item, ',')) {
        size_t start = item.find_first_not_of(" \t");
        size_t end = item.find_last_not_of(" \t");
        if (start != std::string::npos && end != std::string::npos)
            result.push_back(item.substr(start, end - start + 1));
        else if (start != std::string::npos)
            result.push_back(item.substr(start));
        else
            result.push_back("");
    }
    return result;
}

void Server::handleInvite(size_t i, int client_fd, const std::vector<std::string> &tokens) {
    Client &client = clients[i - 1];
    const std::string &nickname = client.getNickname();

    if (tokens.size() < 3 || tokens[2].empty()) {
        sendError(client_fd, 461, nickname, "", "Not enough parameters.");
        return;
    }

    std::string target_nicks = tokens[1];
    std::string channel_name = tokens[2];

    if (!isValidKickChannelName(channel_name)) {
        sendError(client_fd, 403, nickname, channel_name, "No such channel");
        return;
    }

    Channel *target_channel = findChannelByName(channel_name);
    if (!target_channel) {
        sendError(client_fd, 403, nickname, channel_name, "No such channel");
        return;
    }

    if (!isUserInChannel(target_channel, client)) {
        sendError(client_fd, 442, nickname, channel_name, "You're not on that channel");
        return;
    }

    if (!isUserOperatorOrCreator(target_channel, client)) {
        sendError(client_fd, 482, nickname, "", "You're not a channel operator");
        return;
    }

    std::vector<std::string> users = splitByComma(target_nicks);
    for (size_t idx = 0; idx < users.size(); ++idx) {
        std::string &target_nick = users[idx];
        if (target_nick.empty()) // skip empty entries
            continue;
        Client *target_client = findGlobalClientByNick(target_nick);
        if (!target_client) {
            sendError(client_fd, 401, nickname, target_nick, "No such nick/channel");
            continue;
        }
        sendInviteMessages(target_channel, client, target_client, channel_name, target_nick, client_fd);
    }
}


Client *Server::findGlobalClientByNick(const std::string &target_nick) {
    for (size_t j = 0; j < clients.size(); ++j) {
        if (clients[j].getNickname() == target_nick)
            return &clients[j];
    }
    return NULL;
}

void Server::sendInviteMessages(Channel *channel, Client &inviter, Client *target_client, const std::string &channel_name, const std::string &target_nick, int inviter_fd) {
    std::string invite_msg = ":" + inviter.getNickname() + "!" + inviter.getUsername() + "@" + inviter.getIp() + " INVITE " + target_nick + " " + channel_name + "\r\n";
    send(target_client->getFd(), invite_msg.c_str(), invite_msg.length(), 0);

    channel->add_invited_user(*target_client);

    std::string inviting_msg = ": 341 " + inviter.getNickname() + " " + target_nick + " " + channel_name + "\r\n";
    send(inviter_fd, inviting_msg.c_str(), inviting_msg.length(), 0);
}
