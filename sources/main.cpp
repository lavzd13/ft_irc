#include "../includes/Server.hpp"

int	main(int argc, char **argv) {
	if (argc != 3) {
		std::cerr << "Error: wrong number of arguments\nUsage: " << "<port> <password>" << std::endl;
		return 1;
	}

	if (!check_port(argv[1])) {
		std::cerr << "Error: port number must be valid integer between 1-65535." << std::endl;
		return 1;
	}
	size_t	port = atoi(argv[1]);
	std::string	pass = argv[2];
	if (port < 1 || port > 65535 || pass == "") {
		std::cerr << "Error: port must be between 1-65535 and password can't be empty." << std::endl;
		return 1;
	}

	Server	irc_server(port, pass);
	if (!irc_server.start_server())
		return 1;
	return 0;
}
