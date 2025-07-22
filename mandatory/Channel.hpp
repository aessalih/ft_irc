#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <string>
# include <vector>
# include "Client.hpp"

typedef struct mode {
    int i;  // invite-only
    int t;  // topic restriction
    int k;  // channel key
    int o;  // operator privilege
    int l;  // user limit
} MODE;

class Channel {
    private:
        std::string         name;
        std::string         key; // string
        std::string         topic;
        std::string         mode;
        int                 current_clients;
        int                 max_client;
        std::vector<Client> priveleged_client;
        Client              creator;  // channel creator
        MODE                channel_mode;  // channel mode flags
        // --- mODE FLAGS ---
        bool invite_only;         // +i
        bool topic_restricted;    // +t
        std::vector<Client> operators;
        std::vector<Client> invited_users;
        time_t              topic_set_time; // timestamp for when topic was last set

    public:
        std::vector<Client> clients;
        Channel();  // default constructor
        Channel(std::string name, std::string key);
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
        std::string     get_key() const;
        void            setKey(std::string new_key);
        void            removeKey();
        bool            hasKey() const;
        void            add_client(Client &client);
        void            delete_client(Client &client);
        
        //access methods
        size_t          get_clients_size() const;
        const Client    &get_client(size_t index) const;
        const std::vector<Client>   &get_clients() const;
        const Client    &getCreator() const;  // get channel creator
        void            setCreator(const Client &client);  // set channel creator

        // --- MODE METHODS ---
        void setInviteOnly(bool value);
        bool isInviteOnly() const;

        void setTopicRestricted(bool value);
        bool isTopicRestricted() const;

        void setMaxClients(int max);
        void removeUserLimit();
        bool hasUserLimit() const;

        // operator management
        void addOperator(const Client &client);
        void removeOperator(const Client &client);
        bool isOperator(const Client &client) const;

        // mode management
        void setMode(char mode, int value);
        int getMode(char mode) const;
        void updateModeString();
        const MODE &getChannelMode() const;

         const std::vector<Client> &get_invited_users() const;
        int get_max_clients() const;
        void add_invited_user(const Client &client);
        void remove_invited_user(const Client &client);

        void set_topic_time(time_t t); // set topic time
        time_t get_topic_time() const; // get topic time
};

#endif