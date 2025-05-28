#include "Server.hpp"

void	program_init() {
	for (int i = 0; i < 5; i++) {
		std::cout << RUNNING << std::endl;
		sleep(1);
	}
}

int	check_input(char **av) {
	int	i = 0;
	int	has = 0;

	while (av[2][i]) {
		if (av[2][i] < 33 || av[2][i] > 126) {
			has = 1;
			break;
		}
		i++;
	}
	if (has == 1) {
		std::cerr << "\033[1;31m--THE PASSWORD MUST HAVE PRINTABLE CHARACTER ONLY(SPACE EXCLUDED)--\033[0m\n";
		return -1;
	}
	return (0);
}

int main(int ac, char **av) {
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
