#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <vector>
#include <iostream>
#include <sys/types.h>

class Client {
	private:
		std::string					_user;
		std::string					_real_name;
		std::string					_input_buff;
		std::string					_output_buff;
		std::vector<std::string>	_channels;
		std::string					_client_ip;
		bool						_password_ok;
		bool						_registered;
		bool						_welcomed;
	public:
		std::string					_nick;

		Client();
		void						append_input_buff(const char* input, size_t bytes);
		void						append_output_buff(const std::string& input);
		std::string&				getInputBuff();
		std::string&				getOutputBuff();
		void						flushInputBuff();
		void						flushOutputBuff();
		std::string					getIP() const;
		void						setIP(std::string IP);
		bool						is_registered() const;
		void						register_client();
		void						set_pass(bool);
		bool						get_pass() const;
		void						set_user(const std::string& username, const std::string& realname);
		void						addChannel(std::string& channelName);
		std::vector<std::string>&	getChannels();
		void						removeChannel(const std::string& channel);
		std::string&				getUser();
		std::string					get_user() const;
		bool						isWelcomed() const;
		void						welcomed();
};

#endif
