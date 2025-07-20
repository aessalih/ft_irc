#include "Server.hpp"

int Server::flag = 1;

void	program_init() {
	for (int i = 0; i < 3; i++) {
		std::cout << RUNNING << std::endl;
		sleep(1);
	}
}

int	check_input(char **av) {
	int	i = 0;
	int	has = 0;
	std::string pass = av[2];

	while (av[2][i]) {
		if (av[2][i] < 33 || av[2][i] > 126) {
			has = 1;
			break;
		}
		i++;
	}
	i = 0;
	while (av[1][i]) {
		if (av[1][i] < '0' || av[1][i] > '9') {
			std::cerr << "\033[1;31m--ENTER A VALID PORT NUMBER--\033[0m\n";
			return -1;
		}
		i++;
	}
	int num = std::atoi(av[1]);
	if (num <= 1023 || num > 65535) {
		std::cerr << "\033[1;31m--ENTER A VALID PORT NUMBER [1024-65535]--\033[0m\n";
		return -1;
	}
	if (has == 1 || pass.length() < 1) {
		std::cerr << "\033[1;31m--THE PASSWORD MUST HAVE PRINTABLE CHARACTER ONLY(WHITE SPACES EXCLUDED)--\033[0m\n";
		return -1;
	}
	return (0);
}

void v()
{
	system("lsof -c ircserv");
}

int main(int ac, char **av) {
	atexit(v);
	program_init();
	if (ac != 3) {
		std::cerr << ERROR << std::endl;
		std::cerr << "\033[1;31m--PLEASE ENTER TWO ARGUMENTS--\033[0m\n";
		return 2;
	}
	if (check_input(av) == -1)
		return 2;
	Server ser(av);

	if (ser.init() < 0)
		return -1;
	ser.run();
	return 0;
}
