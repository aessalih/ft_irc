SRC= main.cpp Server.cpp Client.cpp

CC=c++
FLAGS= -Wall -Wextra -Werror -std=c++98 -fsanitize=address -g

O_SRC=$(SRC:.cpp=.o)

NAME=ircserv

%.o: %.cpp Server.hpp Client.hpp
	$(CC) $(FLAGS) -c $< -o $@

all: $(NAME)

$(NAME): $(O_SRC)
	$(CC) $(FLAGS) $(O_SRC) -o $(NAME)

clean:
	rm -f $(O_SRC)

fclean: clean
	rm -f $(NAME)

re: fclean all