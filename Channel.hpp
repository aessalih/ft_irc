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
};

#endif