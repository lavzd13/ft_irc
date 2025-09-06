#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <ctime>
#include <cstdio>
#include <cerrno>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <fcntl.h>
#include <limits.h>
#include <iostream>
#include <unistd.h>
#include "Utils.hpp"
#include "Colors.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "Responses.hpp"

# ifndef DEBUG
#  define DEBUG 0
# endif

#define MAX_CONNECTIONS 1024
class Server {
	private:
		int								_epoll_fd;
		int								_server_fd;
		int								_server_port;
		std::string						_server_pass;
		std::map<int, Client>			_clients;
		std::map<std::string, Channel>	_channels;

		void							process_channels(Client& client, const std::vector<std::string>&, const std::vector<std::string>&);
		void							process_mode_flag(Client&, const std::pair<std::vector<std::string>, std::string>& params, Channel* channel);
		void							process_privmsg(Client& client, const std::map<std::string, std::string>& recipients, const std::string& message);
	public:
		Server(int port, std::string pass);

		void							cleanup_all();
		int								start_server();
		const std::map<int, Client>&	get_clients() const;
		void							cleanup_client(int fd);
		static void						handle_sigint(int sig);
		void							update_EPOLLOUT(int fd);
		void							set_nonblocking_flag(int fd);
		void							send_line(int fd, std::string msg);
		void							channel_erase(std::string& name);
	//	bool							can_register_client(Client& client);
		void							serverLog(const std::string& message, const std::string& style);
		Client*							find_client(const std::string& nickname);
		void							handle_command(int fd, std::string& input);
		int								find_client_fd(const Client& client) const;
		Channel*						find_channel(const std::string& name) const;
		void							currentChannelModes(Client& client, Channel* channel);
		void 							handle_cap(Client&, const std::pair<std::vector<std::string>, std::string>& params);
		void 							handle_mode(Client&, const std::pair<std::vector<std::string>, std::string>& params);
		void							handle_pass(Client&, const std::pair<std::vector<std::string>, std::string>& params);
		void							handle_nick(Client&, const std::pair<std::vector<std::string>, std::string>& params);
		void							handle_user(Client&, const std::pair<std::vector<std::string>, std::string>& params);
		void							handle_join(Client&, const std::pair<std::vector<std::string>, std::string>& params);
		void							handle_part(Client&, const std::pair<std::vector<std::string>, std::string>& params);
		void							handle_quit(Client&, const std::pair<std::vector<std::string>, std::string>& params);
		void							handle_kick(Client&, const std::pair<std::vector<std::string>, std::string>& params);
		void							handle_topic(Client&, const std::pair<std::vector<std::string>, std::string>& params);
		void							handle_invite(Client&, const std::pair<std::vector<std::string>, std::string>& params);
		void							handle_privmsg(Client&, const std::pair<std::vector<std::string>, std::string>& params);
		void							handle_unknown(Client&, const std::pair<std::vector<std::string>, std::string>& params);
		void							handle_ping_pong(Client&, const std::pair<std::vector<std::string>, std::string>& params);
};

typedef void (Server::*CommandHandler)(Client&, const std::pair<std::vector<std::string>, std::string>&);
struct Commands
{
	std::string name;
	CommandHandler handler;
};



#endif
