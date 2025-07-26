M_SRC= mandatory/main.cpp mandatory/Server.cpp mandatory/Client.cpp mandatory/Channel.cpp mandatory/cmds/invite.cpp \
	mandatory/cmds/join.cpp mandatory/cmds/kick.cpp mandatory/cmds/privmsg.cpp mandatory/cmds/topic.cpp mandatory/cmds/mode.cpp

B_SRC= bonus/main.cpp bonus/Board.cpp

CC=c++
FLAGS= -Wall -Wextra -Werror -std=c++98

M_O_SRC=$(M_SRC:.cpp=.o)
B_O_SRC=$(B_SRC:.cpp=.o)

NAME=ircserv

BONUS_NAME=irc_bot

mandatory/%.o: mandatory/%.cpp mandatory/Server.hpp mandatory/Client.hpp mandatory/Channel.hpp
	$(CC) $(FLAGS) -c $< -o $@

bonus/%.o: bonus/%.cpp bonus/Board.hpp
	$(CC) $(FLAGS) -c $< -o $@

all: $(NAME)

bonus: $(BONUS_NAME)

$(NAME): $(M_O_SRC)
	$(CC) $(FLAGS) $(M_O_SRC) -o $(NAME)

$(BONUS_NAME): $(B_O_SRC)
	$(CC) $(FLAGS) $(B_O_SRC) -o $(BONUS_NAME)

clean:
	rm -f $(M_O_SRC) $(B_O_SRC)

fclean: clean
	rm -f $(NAME) $(BONUS_NAME)

re: fclean all
