#include "../includes/Server.hpp"

static volatile sig_atomic_t g_terminate = 0;

Server::Server(int port, std::string pass) {
	_server_pass = pass;
	_server_port = port;
	_server_fd = -1;
	_epoll_fd = -1;
}

void	Server::cleanup_all() {
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		std::string	buffer = "\n\033[1;31m !!! Server is shutting down, your connection will be no longer valid !!! \033[0m";
		send(it->first, buffer.c_str(), buffer.size(), 0);
		close(it->first);
	}
	_clients.clear();
	if (_server_fd != -1)
		close(_server_fd);
	if (_epoll_fd  != -1)
		close(_epoll_fd);
}

// Cleaning up fd before closing connection with client
void	Server::cleanup_client(int fd) {
	for (;;) {
		char buf[4096];
		ssize_t n = recv(fd, buf, sizeof(buf), 0);
		if (n > 0) continue;
		break;
	}
	Client&	temp = _clients[fd];
	int	client_fd = find_client_fd(temp);
	std::vector<std::string> joined_channels = temp.getChannels();
	for (std::vector<std::string>::iterator it = joined_channels.begin(); it != joined_channels.end(); ++it)
	{
		Channel* chan = find_channel(*it);
		if (!chan)
			continue;
		chan->removeMember(client_fd);
		if (chan->getMembers().empty())
			channel_erase(*it);
	}
	epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
	close(fd);
	_clients.erase(fd);
}

void	Server::update_EPOLLOUT(int fd) {
	if (_clients.find(fd) == _clients.end())
		return;
	struct	epoll_event	ev;
	std::memset(&ev, 0, sizeof(ev));
	ev.data.fd = fd;
	ev.events = EPOLLIN | EPOLLRDHUP;
	if (!_clients[fd].getOutputBuff().empty()) {
		ev.events |= EPOLLOUT;
	}
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, fd, &ev) == -1) {
		std::cerr << "Error: epoll_ctl() failed in update_EPOLLOUT(), continuing execution..." << std::endl;
		cleanup_client(fd);
	}
}

void	Server::send_line(int fd, std::string msg) {
	if (_clients.find(fd) == _clients.end())
		return ;
	Client	&temp = _clients[fd];
	std::string	crlf = "\r\n";
	temp.getOutputBuff() += msg;
	temp.getOutputBuff() += crlf;
	update_EPOLLOUT(fd);
}

int Server::find_client_fd(const Client& client) const
{
	for (std::map<int, Client>::const_iterator it = _clients.begin();
		 it != _clients.end(); ++it)
	{
		if (&it->second == &client)
			return (it->first);
	}
	return (-1);
}

Client* Server::find_client(const std::string& nickname)
{
	std::string check_name = to_lower(nickname);
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		Client& current_client = it->second;
		std::string it_name = to_lower(current_client._nick);
		if (check_name == it_name)
			return (&current_client);
	}
	return (NULL);
}

void	Server::handle_command(int fd, std::string& input)
{
	std::string	user_input = input;
	if (user_input.empty() || string_is_space(user_input))
		return ;
	std::string prefix;
	if (*user_input.begin() == ':')
	{
		std::size_t	n = user_input.find_first_of(' ');
		if (n != std::string::npos)
		{
			prefix = user_input.substr(1, n - 1);
			if (prefix.empty())
				return ;
			user_input = user_input.substr(n + 1);
		}
		else
			return ;
	}
	std::string	trailing = "";
	size_t	pos = user_input.find(" :");
	bool has_trailing_colon_only = false;
	if (pos != std::string::npos)
	{
		trailing = user_input.substr(pos + 2);
		user_input = user_input.substr(0, pos);
		if (trailing.empty())
			has_trailing_colon_only = true;
	}
	std::pair<std::vector<std::string>, std::string> tokenized_input;
	std::istringstream	iss(user_input);
	std::string			token;
	while (iss >> token)
		tokenized_input.first.push_back(token);
	if (tokenized_input.first.empty())
		return ;
	std::string	command = to_upper(tokenized_input.first[0]);
	if (tokenized_input.first[0].compare("TOPIC") == 0 && has_trailing_colon_only)
		trailing = "\x01";
	tokenized_input.second = trailing;
	Client	&client = _clients[fd];
	struct Commands commands[13] = { {"PASS", &Server::handle_pass},
	{"NICK", &Server::handle_nick}, {"USER", &Server::handle_user},
	{"JOIN", &Server::handle_join}, {"PRIVMSG", &Server::handle_privmsg},
	{"PING", &Server::handle_ping_pong},{"CAP", &Server::handle_cap},
	{"MODE", &Server::handle_mode}, {"PART", &Server::handle_part},
	{"QUIT", &Server::handle_quit}, {"KICK", &Server::handle_kick},
	{"INVITE", &Server::handle_invite}, {"TOPIC", &Server::handle_topic}};
	for (std::size_t i = 0; i < sizeof(commands)/sizeof(commands[0]); i++)
	{
		if (command == commands[i].name)
			return (this->*commands[i].handler)(client, tokenized_input);
	}
	handle_unknown(client, tokenized_input);
}

// Setting flags on the given FD to non blocking
void	Server::set_nonblocking_flag(int fd) {
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
		std::cerr << "Error: failed to set non blocking flags." << std::endl;
		cleanup_client(fd);
		return;
	}
}

void	Server::handle_sigint(int sig) {
	(void)sig;
	g_terminate = 1;
}

int	Server::start_server() {
	signal(SIGINT, handle_sigint);
	signal(SIGQUIT, handle_sigint);
	_server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (_server_fd == -1) {
		std::cerr << "Error: failed to create socket" << std::endl;
		return 0;
	}

	struct sockaddr_in	address;
	std::memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(_server_port);
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	int	opt = 1;
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		std::cerr << "Call to setsockopt() failed. Continuing execution..." << std::endl;
	}

	if (bind(_server_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
		std::cerr << "Error: failed to bind address to a port." << std::endl;
		cleanup_all();
		return 0;
	}

	if (listen(_server_fd, 128) == -1) {
		std::cerr << "Error: failed to listen on the given port." << std::endl;
		cleanup_all();
		return 0;
	}

	_epoll_fd = epoll_create1(0);
	if (_epoll_fd == -1) {
		std::cerr << "Error: failed to create epoll fd." << std::endl;
		cleanup_all();
		return 0;
	}

	struct epoll_event	server_event;
	std::memset(&server_event, 0, sizeof(server_event));
	server_event.events = EPOLLIN;
	server_event.data.fd = _server_fd;

	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _server_fd, &server_event) == -1) {
		std::cerr << "Error: failed to add server fd to epoll events." << std::endl;
		cleanup_all();
		return 0;
	}

	std::vector<struct epoll_event>	events(128);

	int	retry = 0;
	while (true) {
		if (g_terminate)
			break;
		int	event_count = epoll_wait(_epoll_fd, &events[0], 128, -1);
		if (event_count == -1) {
			if (errno == EINTR)
				continue;
			std::cerr << "Error: failed to call epoll_wait." << std::endl;
			break;
		}
		for (int i = 0; i < event_count; i++) {
			int	fd = events[i].data.fd;
			uint32_t flags = events[i].events;
			retry = 0;
			if (fd == _server_fd) {
				while (true) {
					struct sockaddr_in	client_addr_info;
					std::memset(&client_addr_info, 0, sizeof(client_addr_info));
					socklen_t	size = sizeof(client_addr_info);
					int	client_fd = accept(fd, (struct sockaddr *)&client_addr_info, &size);
					if (client_fd == -1) {
						if (errno == EAGAIN || errno == EWOULDBLOCK)
							break;
						if (errno == EINTR || errno == ECONNABORTED)
							continue;
						std::cerr << "Error: failed to accept new client, continuing execution..." << std::endl;
						break;
					}
					set_nonblocking_flag(client_fd);
					struct epoll_event	client_event;
					client_event.events = EPOLLIN | EPOLLRDHUP;
					client_event.data.fd = client_fd;
					if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) == -1) {
						std::cerr << "Error: failed to add new connection to epoll" << std::endl;
						close(client_fd);
						continue;
					}
					_clients[client_fd] = Client();
					std::string client_ip = inet_ntoa(client_addr_info.sin_addr);
					Client	&temp_client = _clients[client_fd];
					temp_client.setIP(client_ip);
					if (_clients.size() <= MAX_CONNECTIONS) {
						serverLog("New connection from " + client_ip + " address.", BLUE);
						// Sending NOTICE message to the client
						send_line(client_fd, ":ft_irc NOTICE * :Please send PASS, NICK and USER to register");
					}
					else
						send_line(client_fd, ERR_SERVERFULL());
				}
				continue;
			}

			if (flags & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) // EPOLLERR -> error happened on fd EPOLLHUP -> hangup. client is gone EPOLLRDHUP -> closed write side?
			{
				Client	temp_client = _clients[fd];
				serverLog("Client " + temp_client.getIP() + " disconnected.", RED);
				cleanup_client(fd);
				continue;
			}

			if (flags & EPOLLIN) {
				Client	&temp_client = _clients[fd];
				char	temp_buffer[512];
				while (true) {
					ssize_t	bytes = recv(fd, temp_buffer, sizeof(temp_buffer), 0);
					if (bytes > 0) {
						if (bytes >= 512) {
							send_line(fd, ERR_INPUTTOOLONG());
							serverLog("Message too long.", RED);
							temp_client.getInputBuff().clear();
							break;
						}
						temp_client.append_input_buff(temp_buffer, (size_t)bytes);
					}
					else if (bytes == 0) {
						serverLog("Client " + temp_client.getIP() + " disconnected.", RED);
						cleanup_client(fd);
						break;
					}
					else {
						if (errno == EINTR)
							continue;
						if (errno == EAGAIN || errno == EWOULDBLOCK)
							break;
						serverLog("Client " + temp_client.getIP() + " disconnected.", RED);
						cleanup_client(fd);
						break;
					}
				}
				if (_clients.find(fd) != _clients.end()) {
					std::string	&input = temp_client.getInputBuff();
					if (DEBUG)
						std::cout << temp_client._nick + " input: " << input;
					while (true) {
						size_t	crlf = input.find("\r\n");
						size_t	lf = input.find("\n");
						bool	hasCRLF = crlf != std::string::npos;
						bool	hasLF = lf != std::string::npos;
						if (!hasCRLF && !hasLF)
							break;
						size_t	end = 0;
						if (hasCRLF)
							end = crlf;
						else
							end = lf;
						std::string	line = input.substr(0, end);
						if (hasCRLF)
							input.erase(0, end + 2);
						else
							input.erase(0, end + 1);
						// Take the line and handle command
						handle_command(fd, line);
						if (_clients.find(fd) == _clients.end())
							break;
					}
				}
			}
			if ((flags & EPOLLOUT) && (_clients.find(fd) != _clients.end())) {
				Client	&temp_client = _clients[fd];
				while (!temp_client.getOutputBuff().empty()) {
					ssize_t	bytes = send(fd, temp_client.getOutputBuff().data(), temp_client.getOutputBuff().size(), MSG_NOSIGNAL);
					if (bytes > 0) {
						temp_client.getOutputBuff().erase(0,(size_t)bytes);
						continue;
					}
					else if (bytes == -1) {
						if (errno == EINTR) continue;
						else if (errno == EAGAIN || errno == EWOULDBLOCK)
							break;
						else {
							serverLog("Connection to " + temp_client.getIP() + " is lost.", RED);
							cleanup_client(fd);
							break;
						}
					}
					else {
						serverLog("Connection to " + temp_client.getIP() + " is lost.", RED);
						cleanup_client(fd);
						break;
					}
				}
				if (_clients.find(fd) != _clients.end())
					update_EPOLLOUT(fd);
				if (_clients.size() > MAX_CONNECTIONS)
					cleanup_client(fd);
			}
		}
	}
	cleanup_all();
	return 1;
}

const std::map<int, Client>& Server::get_clients() const {
	return this->_clients;
}


Channel*	Server::find_channel(const std::string& name) const {
	std::string check_name = to_lower(name);
	for (std::map<std::string, Channel>::const_iterator it = this->_channels.begin(); it != this->_channels.end(); it++)
	{
		std::string curr_chan = to_lower(it->first);
		if (check_name == curr_chan)
			return (const_cast<Channel*>(&it->second));}
	return (NULL);
}

void		Server::serverLog(const std::string& message, const std::string& style = "") {
	char buf[20];
	std::time_t now = std::time(NULL);
	std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&now));

	std::cout << BG_BLACK GREEN "[" << buf << "]" <<  RESET BOLD " : " RESET << style << message << RESET << std::endl;
}

void		Server::channel_erase(std::string& name) {
	for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
		if (to_lower(it->first) == to_lower(name)) {
			_channels.erase(it);
			break;
		}
	}
}

