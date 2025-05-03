#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <string>
# include <vector>
# include "Client.hpp"

class Channel {
    private:
        std::string         name;
        int                 key;
        std::string         topic;
        std::string         mode;
        int                 current_clients;
        int                 max_client;
        std::vector<Client> clients;
        std::vector<Client> priveleged_client;
        // --- MODE FLAGS ---
        bool invite_only;         // +i
        bool topic_restricted;    // +t

    public:
        Channel(std::string name, int key);
        Channel(const Channel &other);
        Channel &operator=(const Channel &other);
        ~Channel();
        
        void            set_topic(std::string new_topic);
        std::string     get_topic() const;
        void            set_mode(std::string new_mode);
        std::string     get_mode() const;
        void            set_max_clients(int max);
        void            remove_user_limit();
        std::string     get_name() const;
        int             get_key() const;
        void            set_key(int new_key);
        void            add_client(Client &client);
        void            delete_client(Client &client);
        
        //access methods
        size_t          get_clients_size() const;
        const Client    &get_client(size_t index) const;
        const std::vector<Client>   &get_clients() const;

        // --- MODE METHODS ---
        void setInviteOnly(bool value);
        bool isInviteOnly() const;

        void setTopicRestricted(bool value);
        bool isTopicRestricted() const;

        void setKey(int new_key);
        void removeKey();
        bool hasKey() const;

        void setMaxClients(int max);
        void removeUserLimit();
        bool hasUserLimit() const;

        // Operator management
        void addOperator(const Client& client);
        void removeOperator(const Client& client);
        bool isOperator(const Client& client) const;
};

#endif