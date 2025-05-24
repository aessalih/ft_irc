#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include "Server.hpp"

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
		std::cerr << "--THE PASSWORD MUST HAVE PRINTABLE CHARACTER ONLY(SPACE EXCLUDED)--\n";
		return -1;
	}
	return (0);
}

int main(int ac, char **av) {
	if (ac != 3) {
		std::cerr << "--PLEASE ENTER TWO ARGUMENTS--\n";
		return 2;
	}
	if (check_input(av) == -1)
		return 2;
	// struct sockaddr_in	address;
	// int	sockfd;
	// int	accept_fd;
	// // int	new_socket;
	// int	opt = 1;
	// int	addrlen = sizeof(address);
	// char buffer[1024] = {0};

	// //create a socket
	// sockfd = socket(AF_INET, SOCK_STREAM, 0);
	// if (sockfd == -1) {
	// 	std::cerr << "SOCKET FAILS\n";
	// 	return -1;
	// }
	// setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	// address.sin_family = AF_INET;
	// address.sin_addr.s_addr = INADDR_ANY;
	// address.sin_port = htons(PORT);
	// /*
	// 	socket(): is a system call in network programming that
	// 	creates a new TCP socket in c++ that is defined inside the
	// 	<sys/socket.h> header file.
		
	// 	sockfd: variable that will store the socket file descriptor
	// 	AF_INET: indicates the socket will use the IPv4 address family
	// 	SOCK_STREAM:  specifies tha the socket will use TCP
	// 	0: lets the system choose the default protocol for the 
	// 	specified address family and socket type (TCP in this case).
	// */

	// if (bind(sockfd, (struct sockaddr*)&address, addrlen) == -1) {
	// 	std::cerr << "BIND FAILS\n";
	// 	return -1;
	// }
	// /*
	// 	bind(): is associated with a socket, with a specific local address and port
	// 	number which allows the socket to listen for incoming connection on that
	// 	address
	// 	sockfd: is a file descriptor that represents the socket in your program
	// 	and is used to perform various socket operations
	// 	(struct sockaddr*)&address: casts the address structure to a generic pointer
	// 	type for the bind function.
	// 	sizeof(address): specifies the size of the address structure to inform
	// 	the system how much data to expect.
	// */
	// if (listen(sockfd, 3) == -1) {
	// 	std::cerr << "LISTEN FAILS\n";
	// 	return (-1);
	// }
	// /*
	// 	listen() marks the socket as a passive socket which prepares a 
	// 	socket to accept incoming connection requests (for servers).

	// 	10 is for backlog parameter, which specifies the maximum number of
	// 	pending connections that can be queued while the server is busy.
	// */
	// if ((accept_fd = accept(sockfd, (struct sockaddr*)&address, (socklen_t *)&addrlen)) == -1)
	// {
	// 	std::cerr << "ACCEPT ERROR\n";
	// 	return -1;
	// }
	// /*
	// 	accept(): accepts a new connection from a client (for servers). It 
	// 	extracts the first connection request on the queue of pending connections
	// 	and creates a new socket for that connection.
	// 	(struct sockaddr)&address: this is a type cast that  converts the pointer
	// 	type of clientAddress to a point of type struct sockaddr*.
	// 	&clientLen: It is a pointer to a variable that holds the size of the
	// 	clientAddress.
	// */
	// while (1) {
	// 	if (read(accept_fd, buffer, 1024) == -1)
	// 	{
	// 		std::cerr << "READ FAILS\n";
	// 		close(sockfd);
	// 		close(accept_fd);
	// 		return -1;
	// 	}
	// 	std::cout << "Message from client: " << buffer << std::endl;
	// }
	// close(sockfd);
	// close(accept_fd);
	Server ser(av);

	if (ser.init() < 0)
		return -1;
	ser.run();
	return 0;
}
