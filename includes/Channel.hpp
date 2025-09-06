#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <set>
#include <string>
#include <iostream>

class Server;
class Client;

class Channel {
	private:
		std::string		_topic;
		std::string		_key;
		std::set<int>	_members;
		std::set<int>	_operators;
		std::set<int>	_invite_list;
		bool			_only_invite;
		bool			_locked_topic;
		bool			_is_limited;
		int				_max_members;
	public:
		std::string		_name;
		Server			*_server;

		Channel(std::string name, Server *server);

		std::string		getTopic() const;
		void			setTopic(const std::string& topic);
		std::string		getKey() const;
		void			setKey(const std::string& key);
		bool			keyIsCorrect(const std::string& key) const;
		void			removeKey();
		void			lockTopic();
		void			removeLockTopic();
		bool			isLockedTopic() const;
		bool			addMember(int client_fd);//returns true if channel is not full and member is added successfully
		void			removeMember(int client_fd);
		bool			isMember(int client_fd) const;
		std::set<int>&	getMembers();
		void			addOperator(int client_fd);
		void			removeOperator(int client_fd);
		bool			isOperator(int client_fd) const;
		std::set<int>&	getOperators();
		void			setInviteOnly();
		void			removeInviteOnly();
		bool			isInviteOnlyChannel() const;
		bool			isInvited(int client_fd) const;
		bool			userInvited(Client& client) const;
		void			removeFromInviteList(Client& client);
		void			broadcast(int client_fd, const std::string& message);
		std::string 	getNamesList() const;
		void			inviteUser(int client_fd);
		void			setLimit(std::string& limit);
		void			removeLimit();
		bool			isLimited() const;
		int				getLimit() const;
};

#endif
