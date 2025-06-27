#include "../Server.hpp"

void Server::handleInvite(size_t i, int client_fd, const std::vector<std::string> &tokens) {
    Client &client = clients[i - 1];
    const std::string &nickname = client.getNickname();

    if (tokens.size() < 3 || tokens[2].empty()) {
        sendError(client_fd, 461, nickname, "", "Not enough parameters.");
        return;
    }

    std::string target_nick = tokens[1];
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

    Client *target_client = findGlobalClientByNick(target_nick);
    if (!target_client) {
        sendError(client_fd, 401, nickname, target_nick, "No such nick/channel");
        return;
    }

    sendInviteMessages(target_channel, client, target_client, channel_name, target_nick, client_fd);
}


Client *Server::findGlobalClientByNick(const std::string &target_nick) {
    for (size_t j = 0; j < clients.size(); ++j) {
        if (clients[j].getNickname() == target_nick)
            return &clients[j];
    }
    return NULL;
}

void Server::sendInviteMessages(Channel *channel, Client &inviter, Client *target_client, const std::string &channel_name, const std::string &target_nick, int inviter_fd) {
    std::string invite_msg = ":" + inviter.getNickname() + "!" + inviter.getUsername() + "@127.0.0.1 INVITE " + target_nick + " " + channel_name + "\r\n";
    send(target_client->getFd(), invite_msg.c_str(), invite_msg.length(), 0);

    channel->add_invited_user(*target_client);

    std::string inviting_msg = ": 341 " + inviter.getNickname() + " " + target_nick + " " + channel_name + "\r\n";
    send(inviter_fd, inviting_msg.c_str(), inviting_msg.length(), 0);
}
