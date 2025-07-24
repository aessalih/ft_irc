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

void Server::handleKick(size_t i, int client_fd, const std::vector<std::string> &tokens) {
    Client &client = clients[i - 1];
    const std::string &nickname = client.getNickname();

    if (tokens.size() < 3) {
        sendError(client_fd, 461, nickname, "", "Not enough parameters");
        return;
    }

    std::string channel_name = toLowerCase(tokens[1]);
    std::string target_nicks = tokens[2];

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
        sendError(client_fd, 482, nickname, "", "You're not channel a operator");
        return;
    }

    std::vector<std::string> users = splitByComma(target_nicks);
    for (size_t idx = 0; idx < users.size(); ++idx) {
        std::string &target_nick = users[idx];
        if (target_nick.empty()) // skip empty entries
            continue;
        Client *target_client = findClientInChannel(target_channel, target_nick);
        if (!target_client) {
            sendError(client_fd, 441, nickname, target_nick, "They aren't on that channel");
            continue;
        }
        std::string target_username = target_client->getUsername();
        std::string target_ip = target_client->getIp();
        sendKickMessage(target_channel, client, *target_client, channel_name, target_nick);
        target_channel->delete_client(*target_client);
    }
}

bool Server::isValidKickChannelName(const std::string &name) {
    return (!name.empty() && name[0] == '#');
}


Client *Server::findClientInChannel(Channel *channel, const std::string &target_nick) {
    const std::vector<Client> &channel_clients = channel->get_clients();
    for (size_t j = 0; j < channel_clients.size(); ++j) {
        if (channel_clients[j].getNickname() == target_nick)
            return const_cast<Client*>(&channel_clients[j]);
    }
    return NULL;
}

bool Server::isUserOperatorOrCreator(Channel *channel, Client& client) {
    return (channel->getCreator() == client || channel->isOperator(client));
}

void Server::sendKickMessage(Channel *channel, Client &kicker, Client &target, const std::string &channel_name, const std::string &target_nick) {
    const std::vector<Client> &channel_clients = channel->get_clients();
    (void) target;
    std::string kick_msg = ":" + kicker.getNickname() + "!" + kicker.getUsername() + "@" + kicker.getIp() + " KICK " +
                           channel_name + " " + target_nick + " :Yeet Out\r\n";

    for (size_t j = 0; j < channel_clients.size(); ++j) {
        send(channel_clients[j].getFd(), kick_msg.c_str(), kick_msg.length(), 0);
    }
    // std::cout << "[" << ", " << kick_msg.c_str() << ", "<< kick_msg.length() << "]" << std::endl;
    // send(target.getFd(), kick_msg.c_str(), kick_msg.length(), 0);
}