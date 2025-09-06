#ifndef RESPONSES_HPP
#define RESPONSES_HPP

// Firstly server name and numeric is sent then we send the actual error message
// :<server-name> <numeric> "<client> <parameters...> :<trailing text>"

// ######################################################################  ERROR RESPONSES #########################################################################################################
#define ERR_NEEDMOREPARAMS(command) ":ft_irc 461 * " #command ": " ":Not enough parameters"
#define ERR_NOSUCHCHANNEL(channel) (":ft_irc 403 * " + channel + " :No such channel")
#define ERR_PASSWDMISMATCH() (":ft_irc 464 * :Password incorrect")
#define ERR_CHANOPRIVSNEEDED(nick, channel) (":ft_irc 482 " + nick + " " + channel + " :You're not channel operator")
#define ERR_USERNOTINCHANNEL(nick, channel) (":ft_irc 441 * " + nick + " " + channel + ":" + " :They aren't on that channel")
#define ERR_NOTONCHANNEL(channel) (":ft_irc 442 * " + channel + ":" + " :You're not on that channel")
#define ERR_USERONCHANNEL(nick, channel) (":ft_irc 443 * " + nick + " " + channel + " :is already on channel")
#define ERR_NOSUCHNICK(nick) (":ft_irc 401 * " + nick + " :No such nick")
#define ERR_USERSDONTMATCH() (":ft_irc 502 * :Cant change mode for other users")
#define ERR_UMODEUNKNOWNFLAG() (":ft_irc 501 * :Unkown MODE flag")
#define ERR_ALREADYREGISTRED() (":ft_irc 462 * :You may not reregister")
#define ERR_NONICKNAMEGIVEN() (":ft_irc 431 * :No nickname given")
#define ERR_ERRONEUSNICKNAME() (":ft_irc 432 * :Erroneus nickname")
#define ERR_NICKNAMEINUSE(nick) (":ft_irc 433 * " + nick + " :Nickname is already in use")
#define ERR_NOTREGISTERED() (":ft_irc 451 * :You have not registered")
#define ERR_BADCHANMASK(channel) (":ft_irc 476 * " + channel + " :Bad Channel Mask")
#define ERR_NOTEXTTOSEND() (":ft_irc 412 * :No text to send")
#define ERR_TOOMANYTARGETS() (":ft_irc 407 * :Too many targets")
#define ERR_UNKNOWNCOMMAND(command) (":ft_irc 421 * " + command + ": " + ":Unknown command")
#define ERR_MODEBADCHANNELKEY(nick, user, host, channel, modestring, key, message) (":" + nick + "!" + user + "@" + host + " MODE " + channel + " " + modestring + " " + key + " ::" + message)
#define ERR_BADCHANNELKEY(nick, channel) (":ft_irc 475 " + nick + " " + channel + " :Cannot join channel (+k) - wrong key")
#define ERR_UNKNOWNMODE(mode) (":ft_irc 472 * " + mode + " : is unknown mode char")
#define ERR_CANNOTSENDTOCHAN(channel) (":ft_irc 404 * " + channel + " :Cannot send to channel")
#define ERR_INVITEONLYCHAN(nick, channel) (":ft_irc 473 " + nick + " " + channel + " :Cannot join channel (+i) - you need to be invited")
#define ERR_CHANNELISFULL(nick, channel) (":ft_irc 471 " + nick + " " + channel + " :Cannot join channel (+l) - channel is full")
#define ERR_INPUTTOOLONG() (":ft_irc 417 :Input line was too long")
#define ERR_SERVERFULL() (":ft_irc 503 : Server is full, try again later")
#define ERR_TOOMANYMODEPARAMS(nick, channel, mode) (":ft_irc 696 " + nick + " " + channel + " " + mode + " :Too many parameters for this mode")

// ######################################################################  VALID RESPONSES #########################################################################################################
#define CHANGINGNICKNAME(oldNick, user, host, newNick) (":" + oldNick + "!" + user + "@" + host + " NICK :" + newNick)
#define RPL_INVITING(channel, nick) (":ft_irc 341 * " + nick + " " + channel)
#define RPL_INVITINGCLIENT(nick, user, host, invited_nick, channel) (":" + nick + "!" + user + "@" + host + " INVITE " + invited_nick + " " + channel)
#define RPL_WELCOME(nick, user, host) (":ft_irc 001 " + nick + " :Welcome to the IRC " + user + "@" + host)
// RPL_TOPIC is sent to a client when he joins the channel
#define RPL_TOPIC(nick, channel, topic) (":ft_irc 332 " + nick + " " + channel + " :" + topic)
// RPL_TOPICWHOTIME is sent immediately after RPL_TOPIC, settime is the timestamp when the topic was changed
#define RPL_TOPICWHOTIME(nick, settime) (":ft_irc 333 * " + nick + " " + settime)
#define RPL_JOIN(nick, user, host, channel) (":" + nick + "!" + user + "@" + host + " JOIN :" + channel)
#define RPL_PART(nick, user, host, channel) (":" + nick + "!" + user + "@" + host + " PART " + channel + " :I'm leaving!")
#define RPL_QUIT(nick, user, host, message) (":" + nick + "!" + user + "@" + host + " QUIT :" + message)
#define RPL_PRIVMSG(sender_nick, target, message) (":" + sender_nick + " PRIVMSG " + target + " :" + message)
#define RPL_PRIVMSG_CHAN(sender_nick, user, host, target, message) (":" + sender_nick + "!" + user + "@" + host + " PRIVMSG " + target + " :" + message)
#define MODE_CHANNEL_OP(channel, mode_change, target_nick) (": ft_irc MODE " + channel + " " + mode_change + " " + target_nick)
#define RPL_KICK(source_nick, user, host, channel, target, reason) (":" + source_nick + "!" + user + "@" + host + " KICK " + channel + " " + target + " :" + reason)
#define RPL_NAMREPLY(nick, channel, names) (":ft_irc 353 " + nick + " = " + channel + " :" + names)
#define RPL_ENDOFNAMES(nick, channel) (":ft_irc 366 " + nick + " " + channel + " :End of /NAMES list.")
#define RPL_NOTOPIC(nick, channel) (":ft_irc 331 " + nick + " " + channel + " :No topic is set")
#define RPL_NEWTOPIC(source_nick, user, host, channel, new_topic) (":" + source_nick + "!" + user + "@" + host + " TOPIC " + channel + " :" + new_topic)
#define RPL_CHANMODE(source_nick, user, host, channel, modes, args) (":" + source_nick + "!" + user + "@" + host + " MODE " + channel + " " + modes + (args.empty() ? "" : " " + args))
#define RPL_CHANNELMODEIS(channel, modestring, modeArgs) (":ft_irc 324 " + channel + " " + modestring + " " + modeArgs)

#endif
