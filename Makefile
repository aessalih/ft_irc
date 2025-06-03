SRC= main.cpp Server.cpp Client.cpp Channel.cpp Server_Utils.cpp

CC=c++
FLAGS=  #-std=c++98  -fsanitize=address -g #-Wall -Wextra -Werror 

O_SRC=$(SRC:.cpp=.o)

NAME=ircserv

%.o: %.cpp Server.hpp Client.hpp Channel.hpp xo/Board.h 
	$(CC) $(FLAGS) -c $< -o $@

all: $(NAME)

$(NAME): $(O_SRC)
	$(CC) $(FLAGS) $(O_SRC) -o $(NAME)

clean:
	rm -f $(O_SRC)

fclean: clean
	rm -f $(NAME)

re: fclean all
