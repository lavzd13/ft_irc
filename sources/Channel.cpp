#include "../includes/Channel.hpp"
#include "../includes/Server.hpp"

Channel::Channel(std::string name, Server *server) : _server(server) {
	_name = name;
	_members.clear();
	_operators.clear();
	_invite_list.clear();
	_only_invite = false;
	_locked_topic = false;
	_is_limited = false;
	_max_members = -1;
}

void			Channel::setTopic(const std::string& topic) {
	_topic = topic;
}

std::string		Channel::getTopic() const {
	return _topic;
}

void			Channel::lockTopic() {
	_locked_topic = true;
}

void			Channel::removeLockTopic() {
	_locked_topic = false;
}

bool			Channel::isLockedTopic() const {
	return _locked_topic;
}

bool			Channel::addMember(int client_fd) {
	if (_is_limited)
	{
		if (static_cast<int>(_members.size()) == _max_members)
		{
			const Client& my_client = _server->get_clients().find(client_fd)->second;
			return (_server->send_line(client_fd, ERR_CHANNELISFULL(my_client._nick, _name)), false);
		}
	}
	_members.insert(client_fd);
	return (true);
}

void			Channel::removeMember(int client_fd) {
	_members.erase(client_fd);
	if (isOperator(client_fd))
		removeOperator(client_fd);
}

bool			Channel::isMember(int client_fd) const {
	return (_members.find(client_fd) != _members.end());
}

std::set<int>&	Channel::getMembers() {
	return _members;
}

void			Channel::addOperator(int client_fd) {
	_operators.insert(client_fd);
}

void	Channel::removeOperator(int client_fd) {
	_operators.erase(client_fd);
	if (_operators.empty()) {
		if (!_members.empty()) {
			int new_op_fd = *(_members.begin());
			for (std::set<int>::iterator it = _members.begin(); it != _members.end(); ++it) {
				if (*it != client_fd)
					new_op_fd = *it;
			}
			_operators.insert(new_op_fd);
			if (_server->get_clients().find(new_op_fd) != _server->get_clients().end()) {
				const Client& new_op = _server->get_clients().find(new_op_fd)->second;
				broadcast(client_fd, MODE_CHANNEL_OP(this->_name, "+o", new_op._nick));
			}
		}
	}
}

bool			Channel::isOperator(int client_fd) const {
	return (_operators.find(client_fd) != _operators.end());
}

std::set<int>&	Channel::getOperators() {
	return _operators;
}

void			Channel::setInviteOnly() {
	_only_invite = true;
}

void			Channel::removeInviteOnly() {
	_only_invite = false;
	_invite_list.clear();
}

void			Channel::removeFromInviteList(Client& client) {
	_invite_list.erase(_server->find_client_fd(client));
}


bool			Channel::isInviteOnlyChannel() const {
	return _only_invite;
}

bool			Channel::userInvited(Client& client) const {
	int client_fd = _server->find_client_fd(client);
	return (_invite_list.find(client_fd) != _invite_list.end());
}

void			Channel::setKey(const std::string& key) {
	_key = key;
}

std::string		Channel::getKey() const {
	return _key;
}

bool	Channel::keyIsCorrect(const std::string& key) const
{
	if (_key.empty() || _key == key)
		return true;
	return false;
}

void	Channel::removeKey() {
	_key.clear();
}

void	Channel::broadcast(int client_fd, const std::string& message)
{
	for (std::set<int>::iterator it = this->_members.begin(); it != this->_members.end(); it++)
	{
		if (*it != client_fd)
			_server->send_line(*it, message);
	}
}

bool	Channel::isInvited(int client_fd) const
{
	return _invite_list.find(client_fd) == _invite_list.end() ? false : true;
}

std::string Channel::getNamesList() const
{
	std::string result = "";
	for (std::set<int>::const_iterator it = _members.begin(); it != _members.end(); ++it)
	{
		const Client& c = _server->get_clients().find(*it)->second;
		std::string prefix;
		if (isOperator(*it))
			prefix = "@";
		if (!result.empty())
			result += " ";
		result += prefix + c._nick;
	}
	return (result);
}

void	Channel::inviteUser(int client_fd)
{
	_invite_list.insert(client_fd);
}

void	Channel::setLimit(std::string& limit) {
	_is_limited = true;
	if (limit.size() > 4) {
		if (limit[0] == '-')
			_max_members = 1;
		else
			_max_members = 10000;
		return;
	}
	long	i_limit = std::atol(limit.c_str());
	if (i_limit <= 0)
		i_limit = 1;
	else if (i_limit > 10000)
		i_limit = 10000;
	_max_members = i_limit;
}

void	Channel::removeLimit() {
	_is_limited = false;
	_max_members = -1;
}

bool	Channel::isLimited() const {
	return _is_limited;
}

int		Channel::getLimit() const {
	return _max_members;
}
