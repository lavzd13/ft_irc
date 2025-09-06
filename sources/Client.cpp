#include "../includes/Client.hpp"
#include "../includes/Utils.hpp"

Client::Client() {
	_input_buff = "";
	_output_buff = "";
	_nick = "";
	_user = "";
	_registered = false;
	_password_ok = false;
	_welcomed = false;
}

void	Client::append_input_buff(const char* input, size_t bytes) {
	_input_buff.append(input, bytes);
}

void	Client::append_output_buff(const std::string& output) {
	_output_buff.append(output);
}

void	Client::flushInputBuff() {
	_input_buff.clear();
}

void	Client::flushOutputBuff() {
	_output_buff.clear();
}

std::string&	Client::getInputBuff() {
	return _input_buff;
}

std::string&	Client::getOutputBuff() {
	return _output_buff;
}

std::string	Client::getIP() const {
	return _client_ip;
}

void	Client::setIP(std::string IP) {
	_client_ip = IP;
}

bool	Client::is_registered() const {
	return _registered;
}
void	Client::register_client() {
	if (!is_registered())
		_registered = true;
}

void	Client::set_pass(bool what) {
	_password_ok = what;
}

bool	Client::get_pass() const {
	return _password_ok;
}

void	Client::set_user(const std::string& username, const std::string& realname) {
	if (username.empty() || realname.empty())
		return ;
	_user = username;
	_real_name = realname;
}

std::vector<std::string>&	Client::getChannels() {
	return _channels;
}
std::string		Client::get_user() const {
	return _user;
}

std::string&	Client::getUser() {
	return _user;
}

void			Client::welcomed() {
	_welcomed = true;
}

bool			Client::isWelcomed() const {
	return _welcomed;
}

void	Client::removeChannel(const std::string& channel)
{
	for(std::vector<std::string>::iterator it = _channels.begin(); it != _channels.end();)
	{
		if (to_lower(*it) == to_lower(channel))
			it = _channels.erase(it);
		else
			++it;
	}
}

void	Client::addChannel(std::string& channelName) {
	_channels.push_back(channelName);
}
