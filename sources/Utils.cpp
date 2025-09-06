#include "../includes/Utils.hpp"

bool string_is_space(const std::string& str)
{
	for (std::size_t i = 0; i < str.size(); i++)
	{
		if (str[i] != ' ')
			return false;
	}
	return true;
}

std::string&	to_upper(std::string& str)
{
	std::string	result;
	for (std::string::iterator it = str.begin(); it != str.end(); ++it)
	{
		if (*it == '{')
			result += '[';
		else if (*it == '}')
			result += ']';
		else if (*it == '|')
			result += '\\';
		else if (*it == '^')
			result += '~';
		else
			result += std::toupper(*it);
	}
	str = result;
	return str;
}

std::string		to_lower(const std::string& str)
{
	std::string result;
	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
	{
		if (*it == '[')
			result += '{';
		else if (*it == ']')
			result += '}';
		else if (*it == '\\')
			result += '|';
		else if (*it == '~')
			result += '^';
		else
			result += std::tolower(*it);
	}
	return result;
}

const std::string	to_upper_(const std::string& str)
{
	std::string new_str = str;
	for (std::string::iterator it = new_str.begin(); it != new_str.end(); ++it)
		*it = std::toupper(*it);
	return new_str;
}

bool 	string_is_num(const std::string& str)
{
	for (std::size_t i = 0; i < str.size(); i++)
	{
		if (!std::isdigit(str[i]))
			return false;
	}
	return true;
}

bool	nick_in_use(const std::map<int, Client>& _clients, const std::string& nick)
{
	std::string check_nick = to_lower(nick);
	for (std::map<int, Client>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		std::string reg_nick = to_lower(it->second._nick);
		if (reg_nick == check_nick)
			return true;
	}
	return false;
}

bool	channelname_is_valid(std::string& name)
{
	if (name.size() > 50)
		return false;
	if (name.size() == 1 && name == "0")
		return false;
	if (name.size() == 1)
		return false;
	std::string channame = to_lower(name);
	std::string check = NOT_ALLOWED_CHANNEL;
	unsigned char first = static_cast<unsigned char>(channame[0]);
	if (first != '#' && first != '@')
		return false;
	for (std::size_t i = 1; i < name.size(); i++)
	{
		unsigned char c = static_cast<unsigned char>(channame[i]);
		if (c > 0x7F)
			return false;
		if (!isalnum(c))
		{
			if (check.find(c) != std::string::npos)
				return false;
		}
	}
	return true;
}

bool	channelkey_is_valid(const std::string& key)
{
	if (key.empty() || key.size() > 23)
		return false;
	for (std::size_t i = 0; i < key.size(); i++)
	{
		unsigned char c = static_cast<unsigned char>(key[i]);
		if (c == 0x00 || c == 0x09 || c == 0x0A || c == 0x0B ||
			c == 0x0C || c == 0x0D || c == 0x20 || c > 0x7F)
			return false;
	}
	return true;
}

bool nickname_is_valid(const std::string& nick)
{
	if (nick.empty() || nick.size() > 9)
		return false;
	unsigned char c = static_cast<unsigned char>(nick[0]);
	if (c > 0x7F)
		return false;
	if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')))
	{
		if (!((c >= 0x5B && c <= 0x60) || (c >= 0x7B && c <= 0x7D)))
			return false;
	}
	if (nick.find_first_of(NOT_ALLOWED_NICK) != std::string::npos)
		return false;
	return true;
}


int	check_validity(const std::pair<std::vector<std::string>, std::string>& params) {
	int			argument_needed = 0;
	std::string	modifiers = params.first[2];
	std::string	valid_modifiers = "+-itkol";
	bool		plus = false;

	for (size_t i = 0; i < modifiers.size(); ++i) {
		if (valid_modifiers.find(modifiers[i]) == std::string::npos)
			return -1;
		if (modifiers[i] == '+' || modifiers[i] == '-') {
			if (modifiers[i] == '+')
				plus = true;
			else
				plus = false;
		}
		if (plus && modifiers[i] == 'l')
			argument_needed++;
		if (modifiers[i] == 'o' || modifiers[i] == 'k')
			argument_needed++;
	}
	if (params.first.size() > 3) {
		for (size_t i = 3; i < params.first.size(); ++i)
			argument_needed--;
	}
	if (argument_needed != 0)
		return 0;
	return 1;
}

bool	check_port(const std::string& port) {
	for (size_t i = 0; i < port.size(); ++i)
		if (!std::isdigit(port[i]))
			return false;
	return true;
}
