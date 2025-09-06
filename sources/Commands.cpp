#include "../includes/Server.hpp"

void Server::handle_pass(Client&client, const std::pair<std::vector<std::string>, std::string>& params)
{
	if (client.is_registered())
		return (send_line(find_client_fd(client), ERR_ALREADYREGISTRED()));
	else if (params.first.size() == 1 && params.second.empty())
		return (send_line(find_client_fd(client), ERR_NEEDMOREPARAMS(PASS)));
	else if (params.first.size() == 2 && !params.second.empty())
		return (send_line(find_client_fd(client), ERR_PASSWDMISMATCH()));
	else if (params.first.size() == 2 && params.second.empty())
	{
		if (params.first.back() != this->_server_pass)
			return (send_line(find_client_fd(client), ERR_PASSWDMISMATCH()));
	}
	else if (params.first.size() == 1 && !params.second.empty())
	{
		if (params.second != this->_server_pass)
			return (send_line(find_client_fd(client), ERR_PASSWDMISMATCH()));
	}
	client.set_pass(true);
	if (DEBUG)
		std::cout << "PASS accepted\n";
	if (!client.is_registered() && client.get_pass() && !client.get_user().empty() && !client._nick.empty())
		client.register_client();
	if (client.is_registered() && !client.isWelcomed()) {
		send_line(find_client_fd(client), RPL_WELCOME(client._nick, params.first[1], client.getIP()));
		serverLog("User " + client._nick + " is successfully registered.", GREEN);
		client.welcomed();
	}
}

void	Server::handle_nick(Client&client, const std::pair<std::vector<std::string>, std::string>& params)
{
	if (!client.get_pass() && !_server_pass.empty())
		return send_line(find_client_fd(client), ERR_PASSWDMISMATCH());
	if (params.first.size() <= 1 && params.second.empty())
		return (send_line(find_client_fd(client), ERR_NONICKNAMEGIVEN()));
	if (params.first.size() != 2 || !params.second.empty())
		return (send_line(find_client_fd(client), ERR_ERRONEUSNICKNAME()));
	if (!nickname_is_valid(params.first[1]))
		return (send_line(find_client_fd(client), ERR_ERRONEUSNICKNAME()));
	if (nick_in_use(this->get_clients(), params.first[1]))
		return (send_line(find_client_fd(client), ERR_NICKNAMEINUSE(params.first[1])));
	if (!client._nick.empty())
		send_line(find_client_fd(client), CHANGINGNICKNAME(client._nick, client.getUser(), client.getIP(), params.first[1]));
	client._nick = params.first[1];
	if (!client.is_registered() && client.get_pass() && !client.get_user().empty() && !client._nick.empty())
		client.register_client();
	if (client.is_registered() && !client.isWelcomed()) {
		send_line(find_client_fd(client), RPL_WELCOME(client._nick, params.first[1], client.getIP()));
		serverLog("User " + client._nick + " is successfully registered.", GREEN);
		client.welcomed();
	}
}

void	Server::handle_user(Client&client, const std::pair<std::vector<std::string>, std::string>& params)
{
	if (client.is_registered())
		return (send_line(find_client_fd(client), ERR_ALREADYREGISTRED()));
	if (!client.get_pass() && !_server_pass.empty())
		return send_line(find_client_fd(client), ERR_PASSWDMISMATCH());
	if (params.first.size() != 4 || params.second.empty())
		return (send_line(find_client_fd(client), ERR_NEEDMOREPARAMS(USER)));
	client.set_user(params.first[1], params.second);
	if (!client.is_registered() && client.get_pass() && !client.get_user().empty() && !client._nick.empty())
		client.register_client();
	if (client.is_registered() && !client.isWelcomed()) {
		send_line(find_client_fd(client), RPL_WELCOME(client._nick, params.first[1], client.getIP()));
		serverLog("User " + client._nick + " is successfully registered.", GREEN);
		client.welcomed();
	}
	if (DEBUG)
	{
		std::cout << "USER " << params.first[1] << "\n";
		std::cout << "Real name: " << params.second << "\n";
		std::cout << "USER " << params.first[1] << " is successfully registered\n";
	}
}

void	Server::process_channels(Client& client, const std::vector<std::string>& channels, const std::vector<std::string>& keys)
{
	if (channels[0] == "0")
	{
		if (channels.size() == 1)
		{
			std::vector<std::string> my_channels = client.getChannels();
			if (!my_channels.empty())
			{
				for (std::size_t i = 0; i < my_channels.size(); i++)
				{
					std::vector<std::string> single_channel_vec;
					single_channel_vec.push_back("PART");
					single_channel_vec.push_back(my_channels[i]);
					std::pair<std::vector<std::string>, std::string> params(single_channel_vec, "");
					handle_part(client, params);
				}
			}
			return ;
		}
		else
			return (send_line(find_client_fd(client), ERR_NEEDMOREPARAMS(JOIN)));
	}
	for (std::size_t i = 0; i < channels.size(); i++)
	{
		std::string& channel_name = const_cast<std::string&>(channels[i]);
		std::string key;
		if (i < keys.size())
			key = keys[i];
		Channel *curr = find_channel(channel_name);
		if (!key.empty() && !channelkey_is_valid(key))
		{
			send_line(find_client_fd(client), ERR_BADCHANNELKEY(client._nick, channel_name));
			continue;
		}
		if (curr)
		{
			if (!curr->keyIsCorrect(key))
			{
				send_line(find_client_fd(client), ERR_BADCHANNELKEY(client._nick, channel_name));
				continue;
			}
			else
			{
				if(curr->isMember(find_client_fd(client)))
				{
					send_line(find_client_fd(client), ERR_USERONCHANNEL(client._nick, channel_name));
					continue;
				}
				else if (curr->isInviteOnlyChannel() && !curr->isInvited(find_client_fd(client)))
				{
					send_line(find_client_fd(client), ERR_INVITEONLYCHAN(client._nick, channel_name));
					continue;
				}
				if (curr->addMember(find_client_fd(client)))
				{
					client.addChannel(curr->_name);
					if (curr->isInviteOnlyChannel())
						curr->removeFromInviteList(client);
					curr->broadcast(-1, RPL_JOIN(client._nick, client.getUser(), client.getIP(), curr->_name));
					if (!curr->getTopic().empty())
						send_line(find_client_fd(client), RPL_TOPIC(client._nick, curr->_name, curr->getTopic()));
					send_line(find_client_fd(client), RPL_NAMREPLY(client._nick, curr->_name, curr->getNamesList()));
					send_line(find_client_fd(client), RPL_ENDOFNAMES(client._nick, curr->_name));
					serverLog(client._nick + " joined " + curr->_name + " channel.", MAGENTA);
				}
			}
		}
		else
		{
			if (!channelname_is_valid(channel_name))
			{
				send_line(find_client_fd(client), ERR_BADCHANMASK(channel_name));
				continue;
			}
			_channels.insert(std::make_pair(channel_name, Channel(channel_name, this)));
			Channel& new_channel = _channels.at(channel_name);
			if (!key.empty())
				new_channel.setKey(key);
			client.addChannel(new_channel._name);
			new_channel.addMember(find_client_fd(client));
			new_channel.addOperator(find_client_fd(client));
			new_channel.broadcast(-1, RPL_JOIN(client._nick, client.getUser(), client.getIP(), new_channel._name));
			send_line(find_client_fd(client), RPL_NAMREPLY(client._nick, new_channel._name, new_channel.getNamesList()));
			send_line(find_client_fd(client), RPL_ENDOFNAMES(client._nick, new_channel._name));
			serverLog(client._nick + " created " + new_channel._name + " channel.", MAGENTA);
			if (DEBUG)
				std::cout << client._nick << " created channel " << new_channel._name << std::endl;
		}
	}
}

void Server::handle_join(Client&client, const std::pair<std::vector<std::string>, std::string>&params)
{
	if (!client.is_registered())
		return (send_line(find_client_fd(client), ERR_NOTREGISTERED()));
	if (params.first.size() <= 1 || params.first.size() > 3 || !params.second.empty())
		return (send_line(find_client_fd(client), ERR_NEEDMOREPARAMS(JOIN)));
	std::vector<std::string> chan_names;
	std::size_t n = params.first[1].find(',');
	if (n != std::string::npos)
	{
		std::stringstream ss(params.first[1]);
		std::string item;
		while (std::getline(ss, item, ','))
		chan_names.push_back(item);
	}
	else
	chan_names.push_back(params.first[1]);
	std::vector<std::string> keys;
	if (params.first.size() == 3)
	{		
		std::size_t k = params.first[2].find(',');
		if (k != std::string::npos)
		{
			std::stringstream ss(params.first[2]);
			std::string item;
			while (std::getline(ss, item, ','))
				keys.push_back(item);
		}
		else
			keys.push_back(params.first[2]);
	}
	process_channels(client, chan_names, keys);
}

void	Server::process_privmsg(Client& client, const std::map<std::string, std::string>& recipients, const std::string& message)
{
	for (std::map<std::string, std::string>::const_iterator it = recipients.begin();
		it != recipients.end(); it++)
	{
		if (it->second == "channel")
		{
			Channel *target_channel = find_channel(it->first);
			if (!target_channel)
			{
				send_line(find_client_fd(client), ERR_NOSUCHCHANNEL(it->first));
				continue ;
			}
			if (!target_channel->isMember(find_client_fd(client)))
			{
				send_line(find_client_fd(client), ERR_CANNOTSENDTOCHAN(it->first));
				continue ;
			}
			target_channel->broadcast(find_client_fd(client), RPL_PRIVMSG_CHAN(client._nick, client.getUser(), client.getIP(), target_channel->_name, message));
		}
		else
		{
			Client *target = this->find_client(it->first);
			if (!target)
			{
				send_line(find_client_fd(client), ERR_NOSUCHNICK(it->first));
				continue ;
			}
			send_line(find_client_fd(*target), RPL_PRIVMSG(client._nick, it->first, message));
		}
	}
}

void Server::handle_privmsg(Client&client, const std::pair<std::vector<std::string>, std::string>& params)
{
	if (!client.is_registered())
		return (send_line(find_client_fd(client), ERR_NOTREGISTERED()));
	if (params.first.size() != 2 || params.second.empty() || string_is_space(params.second))
	{
		if (params.second.empty() || string_is_space(params.second))
			return (send_line(find_client_fd(client), ERR_NOTEXTTOSEND()));
		else if (params.first.size() < 2)
			return (send_line(find_client_fd(client), ERR_NEEDMOREPARAMS(PRIVMSG)));
		else
			return (send_line(find_client_fd(client), ERR_TOOMANYTARGETS()));
	}
	std::map<std::string, std::string> split_params;
	std::size_t n = params.first[1].find(',');
	if (n != std::string::npos)
	{
		std::stringstream ss(params.first[1]);
		std::string item;
		while (std::getline(ss, item, ','))
		{
			if (item[0] == '#' || item[0] == '&')
				split_params[item] = "channel";
			else
				split_params[item] = "client";
		}
	}
	else
	{
		if (params.first[1][0] == '#' || params.first[1][0] == '&')
				split_params[params.first[1]] = "channel";
		else
			split_params[params.first[1]] = "client";
	}
	process_privmsg(client, split_params, params.second);
}

void Server::handle_unknown(Client&client, const std::pair<std::vector<std::string>, std::string>& params)
{
	if (find_client_fd(client) == -1)
		return (send_line(find_client_fd(client), ERR_NOTREGISTERED()));
	send_line(find_client_fd(client), ERR_UNKNOWNCOMMAND(params.first[0]));
	(void)params;
}

void	Server::handle_ping_pong(Client& client, const std::pair<std::vector<std::string>, std::string>& params) {
	if (!client.is_registered())
		return (send_line(find_client_fd(client), ERR_NOTREGISTERED()));
	if (params.first[0] == "PING" && !params.second.empty()) {
		return (send_line(find_client_fd(client), "PONG " + params.second));
	}
	else if (params.first[0] == "PING" && params.second.empty()) {
		if (params.first.size() < 2)
			return (send_line(find_client_fd(client), ERR_NEEDMOREPARAMS(PING)));
		return (send_line(find_client_fd(client), "PONG " + params.first[1]));
	}
}

void	Server::handle_cap(Client& client, const std::pair<std::vector<std::string>, std::string>& params) {
	int fd = find_client_fd(client);
	if (fd == -1) return;
	if (params.first.size() > 1) {
		std::string sub = to_upper_(params.first[1]);
		if (sub == "LS")
			return (send_line(fd, "CAP * LS :"));
		if (sub == "END")
			return;
		if (sub == "REQ")
			return (send_line(fd, "CAP * NAK :" + params.second));
	}
	return (send_line(fd, ERR_UNKNOWNCOMMAND(params.first[0])));
}

void	Server::process_mode_flag(Client& client, const std::pair<std::vector<std::string>, std::string>& params, Channel* channel) {
	std::vector<std::string>	args;
	for (size_t i = 3; i < params.first.size(); i++)
		args.push_back(params.first[i]);

	bool	plus = false;
	int		args_counter = 0;
	int		o_counter = 0;
	std::string	flag = params.first[2];
	for (size_t i = 0; i < flag.size(); ++i) {
		if (flag[i] == '+' || flag[i] == '-') {
			if (flag[i] == '-')
				plus = false;
			else
				plus = true;
		}
		std::string	sign = plus ? "+" : "-";
		switch (flag[i]) {
			case 'i':
				if (plus)
					channel->setInviteOnly();
				else
					channel->removeInviteOnly();
				serverLog(client._nick + " applied " + sign + "i MODE to the " + channel->_name + " channel." , CYAN);
				channel->broadcast(-1, RPL_CHANMODE(client._nick, client.getUser(), client.getIP(), channel->_name, sign + "i", std::string("")));
				break;
			case 't':
				if (plus)
					channel->lockTopic();
				else
					channel->removeLockTopic();
				serverLog(client._nick + " applied " + sign + "t MODE to the " + channel->_name + " channel." , CYAN);
				channel->broadcast(-1, RPL_CHANMODE(client._nick, client.getUser(), client.getIP(), channel->_name, sign + "t", std::string("")));
				break;
			case 'k':
				if (args_counter >= (int)args.size())
					return (send_line(find_client_fd(client), ERR_NEEDMOREPARAMS(MODE)));
				if (plus) {
					if (!channelkey_is_valid(args[args_counter])) {
						send_line(find_client_fd(client), ERR_BADCHANNELKEY(client._nick, channel->_name));
						send_line(find_client_fd(client), ERR_MODEBADCHANNELKEY(client._nick, client.getUser(), client.getIP(), channel->_name, "+k", args[args_counter], "Invalid key provided"));
						break;
					}
					channel->setKey(args[args_counter]);
				}
				else {
					if (channel->getKey() == args[args_counter] || channel->getKey().empty())
						channel->removeKey();
					else
						return (send_line(find_client_fd(client), ERR_MODEBADCHANNELKEY(client._nick, client.getUser(), client.getIP(), channel->_name, "-k", args[args_counter], "Wrong key provided")));
				}
				serverLog(client._nick + " applied " + sign + "k MODE to the " + channel->_name + " channel." , CYAN);
				channel->broadcast(-1, RPL_CHANMODE(client._nick, client.getUser(), client.getIP(), channel->_name, sign + "k", std::string(args[args_counter])));
				args_counter++;
				break;
			case 'l':
				if (plus) {
					if (args_counter >= (int)args.size())
						return (send_line(find_client_fd(client), ERR_NEEDMOREPARAMS(MODE)));
					channel->setLimit(args[args_counter]);
					std::ostringstream	oss;
					oss << channel->getLimit();
					serverLog(client._nick + " applied " + sign + "l " + args[args_counter] +" MODE to the " + channel->_name + " channel." , CYAN);
					channel->broadcast(-1, RPL_CHANMODE(client._nick, client.getUser(), client.getIP(), channel->_name, sign + "l", oss.str()));
					args_counter++;
					break;
				}
				channel->removeLimit();
				serverLog(client._nick + " applied " + sign + "l MODE to the " + channel->_name + " channel." , CYAN);
				channel->broadcast(-1, RPL_CHANMODE(client._nick, client.getUser(), client.getIP(), channel->_name, sign + "l", std::string("")));
				break;
			case 'o':
				if (++o_counter > 3) {
					send_line(find_client_fd(client), ERR_TOOMANYMODEPARAMS(client._nick, channel->_name, sign + "o"));
					send_line(find_client_fd(client), ERR_MODEBADCHANNELKEY(client._nick, client.getUser(), client.getIP(), channel->_name, sign + "o", args[args_counter], "Too many parameters for this mode"));
					break;
				}
				if (args_counter >= (int)args.size()) {
					send_line(find_client_fd(client), ERR_MODEBADCHANNELKEY(client._nick, client.getUser(), client.getIP(), channel->_name, sign + "o", args[args_counter], "Not enough parameters"));
					return (send_line(find_client_fd(client), ERR_NEEDMOREPARAMS(MODE)));
				}
				if (find_client(args[args_counter]) == NULL) {
					send_line(find_client_fd(client), ERR_NOSUCHNICK(args[args_counter]));
					send_line(find_client_fd(client), ERR_MODEBADCHANNELKEY(client._nick, client.getUser(), client.getIP(), channel->_name, sign + "o", args[args_counter], "No such nick"));
					args_counter++;
					break;
				}
				if (!channel->isMember(find_client_fd(*find_client(args[args_counter])))) {
					send_line(find_client_fd(client), ERR_USERNOTINCHANNEL(args[args_counter], channel->_name));
					send_line(find_client_fd(client), ERR_MODEBADCHANNELKEY(client._nick, client.getUser(), client.getIP(), channel->_name, sign + "o", args[args_counter], "They aren't on that channel"));
					args_counter++;
					break;
				}
				if (plus)
					channel->addOperator(find_client_fd(*find_client(args[args_counter])));
				else
					channel->removeOperator(find_client_fd(*find_client(args[args_counter])));
				serverLog(client._nick + " applied " + sign + "o " + args[args_counter] + " MODE to the " + channel->_name + " channel." , CYAN);
				channel->broadcast(-1, RPL_CHANMODE(client._nick, client.getUser(), client.getIP(), channel->_name, sign + "o", std::string(args[args_counter])));
				args_counter++;
				break;
			default:
				continue;
		}
	}
}

void	Server::currentChannelModes(Client& client, Channel* channel) {
	std::string	modestring = "+";
	std::string	modeArgs = " ";
	if (channel->isInviteOnlyChannel())
		modestring += 'i';
	if (channel->isLockedTopic())
		modestring += 't';
	if (!channel->getKey().empty()) {
		modestring += 'k';
		modeArgs += channel->getKey() + " ";
	}
	if (channel->isLimited()) {
		modestring += 'l';
		std::ostringstream oss;
		oss << channel->getLimit();
		modeArgs += oss.str() + " ";
	}
	send_line(find_client_fd(client), RPL_CHANNELMODEIS(channel->_name, modestring, modeArgs));
	send_line(find_client_fd(client), RPL_CHANMODE(client._nick, client.get_user(), client.getIP(), channel->_name, modestring, modeArgs));
}

void	Server::handle_mode(Client& client, const std::pair<std::vector<std::string>, std::string>& params) {
	if (!client.is_registered())
		return (send_line(find_client_fd(client), ERR_NOTREGISTERED()));
	int fd = find_client_fd(client);
	Channel*	channel = find_channel(params.first[1]);
	if (channel == NULL)
		return (send_line(fd, ERR_NOSUCHCHANNEL(params.first[1])));
	if (params.first.size() < 3) {
		return (currentChannelModes(client, channel));
	}
	if (!channel->isMember(fd)) {
		return (send_line(fd, ERR_NOTONCHANNEL(params.first[1])));
	}
	if (!channel->isOperator(fd)) {
		return (send_line(fd, ERR_CHANOPRIVSNEEDED(client._nick, params.first[1])));
	}
	int	check = check_validity(params);
	if (check < 1) {
		if (check == 0) {
			send_line(fd, ERR_MODEBADCHANNELKEY(client._nick, client.getUser(), client.getIP(), channel->_name, params.first[2], "", "Invalid number of parameters"));
			return (send_line(fd, ERR_NEEDMOREPARAMS(MODE)));
		}
		else {
			send_line(fd, ERR_MODEBADCHANNELKEY(client._nick, client.getUser(), client.getIP(), channel->_name, params.first[2], "", "Unkown MODE"));
			return (send_line(fd, ERR_UNKNOWNMODE(params.first[2])));
		}
	}
	process_mode_flag(client, params, channel);
}

void	Server::handle_part(Client& client, const std::pair<std::vector<std::string>, std::string>& params)
{
	if (!client.is_registered())
		return (send_line(find_client_fd(client), ERR_NOTREGISTERED()));
	if (params.first.size() < 2)
		return (send_line(find_client_fd(client), ERR_NEEDMOREPARAMS(PING)));
	std::vector<std::string>	leave_channels;
	std::size_t n = params.first[1].find(',');
	if (n != std::string::npos)
	{
		std::stringstream ss(params.first[1]);
		std::string item;
		while (std::getline(ss, item, ','))
			leave_channels.push_back(item);
	}
	else
		leave_channels.push_back(params.first[1]);
	for (std::vector<std::string>::iterator it = leave_channels.begin(); it != leave_channels.end(); it++)
	{
		Channel * leave = find_channel(*it);
		if (!leave)
		{
			send_line(find_client_fd(client), ERR_NOSUCHCHANNEL(*it));
			continue ;
		}
		else if (!leave->isMember(find_client_fd(client)))
		{
			send_line(find_client_fd(client), ERR_NOTONCHANNEL(*it));
			continue ;
		}
		serverLog(client._nick + " left " + leave->_name + " channel.", YELLOW);
		leave->broadcast(-1, RPL_PART(client._nick, client.getUser(), client.getIP(), *it));
		leave->removeMember(find_client_fd(client));
		if (leave->getMembers().empty())
			channel_erase(*it);
		client.removeChannel(*it);
	}
}

void Server::handle_quit(Client& client, const std::pair<std::vector<std::string>, std::string>& params)
{
	int client_fd = find_client_fd(client);
	if (client_fd == -1)
		return send_line(client_fd, ERR_NOTREGISTERED());
	std::string quit_msg = "Client Quit";
	if (!params.second.empty())
		quit_msg = params.second;
	serverLog("Client " + client.getIP() + " (" + client._nick + ") quit: " + quit_msg, RED);
	std::vector<std::string> joined_channels = client.getChannels();
	for (std::vector<std::string>::iterator it = joined_channels.begin(); it != joined_channels.end(); ++it)
	{
		Channel* chan = find_channel(*it);
		if (!chan)
			continue;
		chan->removeMember(client_fd);
		chan->broadcast(client_fd, RPL_QUIT(client._nick, client.getUser(), client.getIP(), quit_msg));
		if (chan->getMembers().empty())
			channel_erase(*it);
	}
	if (params.second.empty())
		cleanup_client(client_fd);
}

void	Server::handle_kick(Client& client, const std::pair<std::vector<std::string>, std::string>& params)
{
	if (!client.is_registered())
		return send_line(find_client_fd(client), ERR_NOTREGISTERED());
	if (params.first.size() != 3)
		return (send_line(find_client_fd(client), ERR_NEEDMOREPARAMS(KICK)));
	std::vector<std::string>	kick_channels;
	std::size_t n = params.first[1].find(',');
	if (n != std::string::npos)
	{
		std::stringstream ss(params.first[1]);
		std::string item;
		while (std::getline(ss, item, ','))
			kick_channels.push_back(item);
	}
	else
		kick_channels.push_back(params.first[1]);
	std::vector<std::string>	kick_users;
	std::size_t k = params.first[2].find(',');
	if (k != std::string::npos)
	{
		std::stringstream ss(params.first[2]);
		std::string item;
		while (std::getline(ss, item, ','))
			kick_users.push_back(item);
	}
	else
		kick_users.push_back(params.first[2]);
	if (kick_channels.size() != kick_users.size())
	{
		if (kick_channels.size() != 1)
			return (send_line(find_client_fd(client), ERR_NEEDMOREPARAMS(KICK)));
		else
		{
			Channel *my_channel = find_channel(kick_channels[0]);
			if (!my_channel)
				return (send_line(find_client_fd(client), ERR_NOSUCHCHANNEL(params.first[1])));
			for (std::vector<std::string>::iterator it = kick_users.begin(); it != kick_users.end(); it++)
			{
				Client *target = find_client(*it);
				if (!target)
				{
					send_line(find_client_fd(client), ERR_NOSUCHNICK(*it));
					continue;
				}
				if (!my_channel->isMember(find_client_fd(*target)))
				{
					send_line(find_client_fd(client), ERR_USERNOTINCHANNEL(target->_nick, my_channel->_name));
					continue;
				}
				std::string reason = client._nick;
				if (!params.second.empty())
					reason = params.second;
				serverLog(client._nick + " kicked " + target->_nick + " from the " + my_channel->_name + " channel.", YELLOW);
				my_channel->broadcast(-1, RPL_KICK(client._nick, client.getUser(), client.getIP(), my_channel->_name, target->_nick, reason));
				my_channel->removeMember(find_client_fd(*target));
				target->removeChannel(my_channel->_name);
				if (my_channel->getMembers().empty())
					this->channel_erase(my_channel->_name);
			}
		}
	}
	else
	{
		for (std::size_t i = 0; i < kick_channels.size(); i++)
		{
			Channel *my_channel = find_channel(kick_channels[i]);
			if (!my_channel)
			{
				send_line(find_client_fd(client), ERR_NOSUCHCHANNEL(kick_channels[i]));
				continue;
			}
			if (!my_channel->isMember(find_client_fd(client)))
			{
				send_line(find_client_fd(client), ERR_NOTONCHANNEL(my_channel->_name));
				continue;
			}
			if (!my_channel->isOperator(find_client_fd(client)))
			{
				send_line(find_client_fd(client), ERR_CHANOPRIVSNEEDED(client._nick, my_channel->_name));
				continue;
			}
			Client *target = find_client(kick_users[i]);
			if (!target)
			{
				send_line(find_client_fd(client), ERR_NOSUCHNICK(kick_users[i]));
				continue;
			}
			if (!my_channel->isMember(find_client_fd(*target)))
			{
				send_line(find_client_fd(client), ERR_USERNOTINCHANNEL(target->_nick, my_channel->_name));
				continue;
			}
			std::string reason = client._nick;
			if (!params.second.empty())
				reason = params.second;
			serverLog(client._nick + " kicked " + target->_nick + " from the " + my_channel->_name + " channel.", YELLOW);
			my_channel->broadcast(-1, RPL_KICK(client._nick, client.getUser(), client.getIP(), my_channel->_name, target->_nick, reason));
			my_channel->removeMember(find_client_fd(*target));
			target->removeChannel(my_channel->_name);
			if (my_channel->getMembers().empty())
				this->channel_erase(my_channel->_name);
		}
	}
}

void	Server::handle_topic(Client& client, const std::pair<std::vector<std::string>, std::string>& params)
{
	if (!client.is_registered())
		return send_line(find_client_fd(client), ERR_NOTREGISTERED());
	if (params.first.size() != 2)
		return (send_line(find_client_fd(client), ERR_NEEDMOREPARAMS(TOPIC)));
	Channel *my_channel = find_channel(params.first[1]);
	if (!my_channel)
		return (send_line(find_client_fd(client), ERR_NOSUCHCHANNEL(params.first[1])));
	if (!my_channel->isMember(find_client_fd(client)))
		return (send_line(find_client_fd(client), ERR_NOTONCHANNEL(my_channel->_name)));
	if (params.second.empty())
	{
		if (!my_channel->getTopic().empty())
			return (send_line(find_client_fd(client), RPL_TOPIC(client._nick, my_channel->_name, my_channel->getTopic())));
		else
			return (send_line(find_client_fd(client), RPL_NOTOPIC(client._nick, my_channel->_name)));
	}
	if (my_channel->isLockedTopic() && !my_channel->isOperator(find_client_fd(client)))
		return (send_line(find_client_fd(client), ERR_CHANOPRIVSNEEDED(client._nick, my_channel->_name)));
	if (params.second == "\x01")
		my_channel->setTopic("");
	else
		my_channel->setTopic(params.second);
	serverLog(client._nick + " changed topic of " + my_channel->_name + " channel to: " + my_channel->getTopic(), MAGENTA);
	my_channel->broadcast(-1, RPL_NEWTOPIC(client._nick, client.getUser(), client.getIP(), my_channel->_name, my_channel->getTopic()));
}

void	Server::handle_invite(Client& client, const std::pair<std::vector<std::string>, std::string>& params)
{
	if (!client.is_registered())
		return send_line(find_client_fd(client), ERR_NOTREGISTERED());
	if (params.first.size() != 3)
		return (send_line(find_client_fd(client), ERR_NEEDMOREPARAMS(INVITE)));
	Channel *my_channel = find_channel(params.first[2]);
	if (!my_channel)
		return (send_line(find_client_fd(client), ERR_NOSUCHCHANNEL(params.first[2])));
	if (!my_channel->isMember(find_client_fd(client)))
		return (send_line(find_client_fd(client), ERR_NOTONCHANNEL(my_channel->_name)));
	if (my_channel->isInviteOnlyChannel() && !my_channel->isOperator(find_client_fd(client)))
		return (send_line(find_client_fd(client), ERR_CHANOPRIVSNEEDED(client._nick, my_channel->_name)));
	Client *target = find_client(params.first[1]);
	if (!target)
		return (send_line(find_client_fd(client), ERR_NOSUCHNICK(params.first[1])));
	if (my_channel->isMember(find_client_fd(*target)))
		return (send_line(find_client_fd(client), ERR_USERONCHANNEL(target->_nick, my_channel->_name)));
	my_channel->inviteUser(find_client_fd(*target));
	serverLog(client._nick + " invited " + target->_nick + " to " + my_channel->_name + " channel.", std::string(ITALIC));
	send_line(find_client_fd(client), RPL_INVITING(my_channel->_name, target->_nick));
	send_line(find_client_fd(*target), RPL_INVITINGCLIENT(client._nick, client.getUser(), client.getIP(), target->_nick, my_channel->_name));
}
