#pragma once
#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <stdbool.h>
#include <string>
#include <cctype>
#include <map>
#include "Client.hpp"
#define NOT_ALLOWED_CHANNEL " ,:"
#define NOT_ALLOWED_NICK " #.,@!:*"

bool 				string_is_space(const std::string& str);
bool 				string_is_num(const std::string& str);
bool				nick_in_use(const std::map<int, Client>& _clients, const std::string& nick);
std::string&		to_upper(std::string& str);
std::string			to_lower(const std::string& str);
const std::string	to_upper_(const std::string& str);
bool				nickname_is_valid(const std::string& nick);
bool				channelname_is_valid(std::string& name);
bool				channelkey_is_valid(const std::string& key);
int					check_validity(const std::pair<std::vector<std::string>, std::string>& params);
bool				check_port(const std::string& port);

#endif
