#include "../inc/header.hpp"
// #include "../inc/Channel.hpp"
#include "../inc/Server.hpp"


std::string message_formatter2(int error, std::string command, const char* message)
{
	std::string msg;
	std::ostringstream oss;
	oss << error;
	msg  = ":server" + oss.str() + ' ' + command + " :" + message;
	return (msg);
}

int		Server::channelAdder(std::string& channelName, User& user, std::string& pass)
{
	std::vector<Channel>::iterator channelIterator = _channels.begin();
	while (channelIterator != _channels.end())
	{
		std::cout << "searching trough channels" << std::endl;
		if (channelName == channelIterator->getName())
		{
			if (channelIterator->getInviteOnly())
			{
				// TODO rimuovere prints per debug
				std::cout << "channel found" << std::endl;
				std::vector<User>::iterator x = std::find(channelIterator->getInvitedUsersVector().begin(), channelIterator->getInvitedUsersVector().end(), user);
				if (x == channelIterator->getInvitedUsersVector().end())
				{
					std::cout << YELLOW;
					std::cout << MAGENTA << x->getNick() << " " << x->getFd() << RESET << std::endl;
					std::string tmp(message_formatter(473, user.getNick(), channelIterator->getName(), "Cannot join channel (+i)"));
					send(user.getFd(), tmp.c_str(), tmp.size(), 0);
					std::cout << RED << user.getNick() << " cannot join channel: invite only restriction" << RESET << std::endl;
					return 1;
				}
				else
					std::cout << BLUE << user.getNick() << " joined channel with restriction" << RESET << std::endl;
			}
			channelIterator->addUserToChannel(user, pass);
			
			// Standard IRC Replies for JOIN
			std::string join_msg = ":" + user.getNick() + " JOIN #" + channelName + "\r\n";
			channelIterator->writeToChannel(join_msg);
			std::string topic_msg = ":server 332 " + user.getNick() + " #" + channelName + " :" + channelIterator->getTopic() + "\r\n";
			send(user.getFd(), topic_msg.c_str(), topic_msg.size(), 0);
			std::string users_list = channelIterator->getNickList();
			std::string namreply_msg = ":server 353 " + user.getNick() + " = #" + channelName + " :" + users_list + "\r\n";
			channelIterator->writeToChannel(namreply_msg);
			std::string endofnames_msg = ":server 366 " + user.getNick() + " #" + channelName + " :End of /NAMES list.\r\n";
			channelIterator->writeToChannel(endofnames_msg);
			return (0);
		}
		++channelIterator;
	}
	return -1;
	
}

void    Server::channelCreate(std::string& channelName, std::string& pass, User& user)
{
	std::string topic;
	// name, pass, creator, topic, max users, invite_only, topic restriction
	Channel new_channel(channelName, pass, user, topic, -1, 0, 0);
	_channels.push_back(new_channel);
	// Standard IRC Replies for successful channel creation and join
    std::string join_msg = ":" + user.getNick() + " JOIN #" + channelName + "\r\n";
    send(user.getFd(), join_msg.c_str(), join_msg.size(), 0);
    std::string topic_msg = ":server 332 " + user.getNick() + " #" + channelName + " :" + topic + "\r\n";
    send(user.getFd(), topic_msg.c_str(), topic_msg.size(), 0);
    std::string namreply_msg = ":server 353 " + user.getNick() + " = #" + channelName + " :" + user.getNick() + "\r\n";
    send(user.getFd(), namreply_msg.c_str(), namreply_msg.size(), 0);
    std::string endofnames_msg = ":server 366 " + user.getNick() + " #" + channelName + " :End of /NAMES list.\r\n";
    send(user.getFd(), endofnames_msg.c_str(), endofnames_msg.size(), 0);
}

int Server::cmdPart(std::vector<std::string> parsed_message, User &user)
{
    //User *user = getUserByFd(user.getFd());
    // if (user.getNick().empty()) {
    //     return 1; // User not found
    // }
	// reference always initialized

    std::string channelName;
    // oss >> channelName;

    if (parsed_message.size() < 2)
    {
        // ERR_NEEDMOREPARAMS (461)
        std::string msg = ":server 461 " + user.getNick() + " PART :Not enough parameters\r\n";
        send(user.getFd(), msg.c_str(), msg.size(), 0);
        return 1;
    }
	else
	{
		channelName = parsed_message[1];
	}

    if (channelName[0] == '#') {
        channelName = channelName.substr(1);
    }

    Channel* targetChannel = findChannelByName(channelName);

    if (targetChannel == NULL) {
        // ERR_NOSUCHCHANNEL (403)
        std::string msg = ":server 403 " + user.getNick() + " #" + channelName + " :No such channel\r\n";
        send(user.getFd(), msg.c_str(), msg.size(), 0);
        return 1;
    }

    if (!isInVector(user, targetChannel->getUserVector())) {
        // ERR_NOTONCHANNEL (442)
        std::string msg = ":server 442 " + user.getNick() + " #" + channelName + " :You're not on that channel\r\n";
        send(user.getFd(), msg.c_str(), msg.size(), 0);
        return 1;
    }

	std::string reason;
	if (parsed_message.size() == 3)
	{
		reason = parsed_message[2];
	}
	else
	{
		reason = "Leaving";
	}

    // Correctly parse the multi-word reason
    // std::getline(oss, reason);
    // if (!reason.empty() && reason[0] == ' ') {
    //     reason = reason.substr(1);
    // }
    // if (!reason.empty() && reason[0] == ':'){
    //     reason = reason.substr(1);
    // } else if (reason.empty()) {
    //     reason = "Leaving"; // Default reason
    // }

    // Remove the user from the channel's internal list
	// moved reply message logic to partUser()
    targetChannel->partUser(user, *targetChannel, reason, PART);

    return 0;
}

int Server::cmdJoin(std::vector<std::string>& mess, User &user)
{
	std::string channelName;
	std::string pass;

	//std::cout << "detected command JOIN" << mess[0] << std::endl;
	mess.erase(mess.begin());
	channelName = mess[0];
	if (mess[0].empty())
	{
		std::string tmp(message_formatter2(461, "JOIN", "Not enough parameters"));
		send(user.getFd(), tmp.c_str(), tmp.size(), 0);
		return 1;
	}
	if (mess[0][0] != '#')
	{
		std::string tmp(message_formatter2(403, "JOIN", "No such channel"));
		send(user.getFd(), tmp.c_str(), tmp.size(), 0);
		return 1;
	}
	channelName = channelName.substr(1);
	mess.erase(mess.begin());
	if (mess.size())
	{
		pass = mess[0];
		mess.erase(mess.begin());
	}
	int result = channelAdder(channelName, user, pass);
	if (result == 1)
		return 1;
	else if (result == 0)
		return 0;
	std::cout << channelName << " channel not found, creating..." << std::endl;
	channelCreate(channelName, pass, user);
	return 0;
}

int		Server::cmdPrivateMsg(std::vector<std::string> parsed_message, User &user)
{
	std::string targetsToken;
	std::string msgBody;
	std::string target;
	std::stringstream tss(targetsToken);
	
	if (parsed_message.size() < 3)
	{
		std::string err = ":server 461 " + user.getNick() + " PRIVMSG :Not enough parameters\r\n";
	 	send(user.getFd(), err.c_str(), err.size(), 0);
	 	return (1);		
	}
	else
	{
		targetsToken = parsed_message[1];
		msgBody = parsed_message[2];
	}

	while (std::getline(tss, target, ','))
	{
		bool is_channel = false;
		if (target.empty())
			return (1);
		size_t i = 0;
 
		if (target[0] == '#')
		{
			is_channel = true;
			std::cout << "channels_no: " << _channels.size() << " i: " << i << std::endl;
			std::string channelName = target.substr(1);
			while (i < _channels.size())
			{
				std::cout << "channel: " << _channels[i].getName() << " i: " << i << std::endl;
				if (_channels[i].getName() == channelName)
				{
					// recipFd = _users[ui].getFd();
					break;
				}
				i++;
			}
			if (i == _channels.size())
			{
				//std::string err = "401 " + findNickName(clientSocket) + " " + singleTarget + " :No such nick/channel\r\n";
				//send(clientSocket, err.c_str(), err.size(), 0);
				std::cerr << "channel not found" << std::endl;
				return (1);
			}
		}
		else
		{
			while (i < _users.size())
			{
				if (_users[i].getNick() == target)
				{
					// recipFd = _users[ui].getFd();
					break;
				}
				i++;
			}
			if (i == _users.size())
			{
				//std::string err = "401 " + findNickName(clientSocket) + " " + singleTarget + " :No such nick/channel\r\n";
				//send(clientSocket, err.c_str(), err.size(), 0);
				std::cerr << "user not found" << std::endl;
				return (1);
			}
		}
		// poll_fds[recipFd]
		// build and send the PRIVMSG to the recipient
		std::string out = ":" + user.getNick() + " PRIVMSG " + target + " :" + msgBody + "\r\n";

		if (is_channel)
			_channels[i].writeToChannel(out);
		else
			send(_users[i].getFd(), out.c_str(), out.size(), 0);
	}
    return (0);
}

int		Server::cmdInvite(std::vector<std::string> parsed_message, User &user)
{
	std::string targetNick;
	std::string channelName;
	//oss >> targetNick >> channelName;

	if (parsed_message.size() < 3)
	{
		std::string err = ":server 461 " + user.getNick() + " INVITE :Not enough parameters\r\n";
	 	send(user.getFd(), err.c_str(), err.size(), 0);
	 	return (1);				
	}

	targetNick = parsed_message[1];
	channelName = parsed_message[2];

	//User inviter = getUserByFd(clientSocket);
	User targetUser = findUserByNick(targetNick);
	if (channelName[0] == '#')
	{
        channelName = channelName.substr(1);
    }
	Channel *targetChannel = findChannelByName(channelName);

	// if (targetNick.empty() || channelName.empty())
	// {
	// 	// ERR_NEEDMOREPARAMS (461)
	// 	std::cout << "wrong number of arguments" << std::endl;
	// 	return (1);
	// }
	
	if (targetUser.getNick().empty())
	{
		std::cout << "user not found" << std::endl;
		//"err 401"
		return (1);
	}

	std::cout << "found user: " << targetUser.getNick() << std::endl;
	std::cout << channelName << std::endl;

	if (targetChannel == NULL)
	{
		std::cout << "channel not found" << std::endl;
		// "err 403"
		return (1);
	}

	// checks

	// Check if inviter is on the channel
    if (!isInVector(user, targetChannel->getUserVector())) {
        std::string err = ":server 442 " + user.getNick() + " #" + channelName + " :You're not on that channel\r\n";
        send(user.getFd(), err.c_str(), err.size(), 0);
        return 1;
    }

	// Check if target is already on the channel
    if (isInVector(targetUser, targetChannel->getUserVector())) {
        std::string err = ":server 443 " + user.getNick() + " " + targetNick + " #" + channelName + " :is already on channel\r\n";
        send(user.getFd(), err.c_str(), err.size(), 0);
        return 1;
    }

	// send invite
	targetChannel->addToInvited(targetUser);

	// Send RPL_INVITING to user
    std::string inviting_msg = ":server 341 " + user.getNick() + " " + targetNick + " #" + channelName + "\r\n";
    send(user.getFd(), inviting_msg.c_str(), inviting_msg.size(), 0);

    // Send INVITE to target user
    std::string invite_msg = ":" + user.getNick() + " INVITE " + targetNick + " :#" + channelName + "\r\n";
    send(targetUser.getFd(), invite_msg.c_str(), invite_msg.size(), 0);


	return (0);
}

int		Server::cmdTopic(std::vector<std::string> parsed_message, User &user)
{
	std::string channel_name;
	//oss >> channel_name;
	std::cout << "detected command TOPIC" << std::endl;
	std::string arg2;
	//oss >> arg2;
	//User targetUser = getUserByFd(clientSocket);

	if (parsed_message.size() < 2)
	{
		std::string err = ":server 461 " + user.getNick() + " TOPIC :Not enough parameters\r\n";
	 	send(user.getFd(), err.c_str(), err.size(), 0);
	 	return (1);						
	}

	channel_name = parsed_message[1];

	if (channel_name.empty())
	{
		std::cout << "fatal error, no channel topic" << std::endl;
		return (1);
	}
	std::cout << channel_name << std::endl;

	if (removeInitialHash(&channel_name))
	{
		std::cout << "bad formatted arguments, need channel" << std::endl;
	}
	else
	{
		std::cout << "hash removed correctly" << std::endl;
		std::cout << channel_name << std::endl;
	}

	Channel *targetChannel = findChannelByName(channel_name);
	if (!targetChannel)
	{
		std::cout << "fatal error, no channel found" << std::endl;
		exit(1);
	}
	targetChannel->showChannelTopic();

	if (!arg2.empty())
	{
		if (targetChannel->isOperatorUser(user))
		{
			std::cout << "user is operator, TOPIC operation allowed" << std::endl;
			targetChannel->setTopic(arg2);
		}
		else
		{
			std::cout << "User is not operator, operation aborted" << std::endl;
			return (1);
		}
	}
	return (0);
}

int	Server::checkCmdMode(std::vector<std::string>& msg_parsed, User& user, Channel* targetChannel, std::string& channelName)
{
	if (targetChannel == NULL)
	{
		std::cout << RED << channelName << " no such channel" << RESET << std::endl;
		std::string mode_err = message_formatter(403, user.getNick(), channelName, "No such channel");
		send(user.getFd(), mode_err.c_str(), mode_err.size(), 0);
		return 1;
	}
	if (!isInVector(user, targetChannel->getUserOperatorsVector()))
	{
		std::cout << RED << user.getNick() << " not an operator" << RESET << std::endl;
		std::string mode_err = message_formatter(482, user.getNick(), channelName, "You're not channel operator");
		send(user.getFd(), mode_err.c_str(), mode_err.size(), 0);
		return 1;
	}
	msg_parsed.erase(msg_parsed.begin());
	if (msg_parsed.size() == 0)
	{
		std::cout << RED << channelName << " flag not present" << RESET << std::endl;
		std::string mode_err = "461 " + user.getNick() + " MODE: need more params";
		send(user.getFd(), mode_err.c_str(), mode_err.size(), 0);
		return 1;
	}
	std::string flag = msg_parsed[0];
	std::string mode[11] = {"+b", "+i", "-i", "+k", "-k", "+o", "-o", "+l", "-l", "+t", "-t"};
	if (std::find(mode, mode+11, flag) == mode + 11)
	{
		std::cout << RED << channelName << " unknown flag" << RESET << std::endl;
		std::string mode_err = "501 " + user.getNick() + " :Unknown MODE flag";
		send(user.getFd(), mode_err.c_str(), mode_err.size(), 0);
		return 1;
	}
	return 0;
}

int Server::cmdMode(std::vector<std::string>& msg_parsed, User& user)
{
	std::cout << "detected command MODE" << std::endl;
	msg_parsed.erase(msg_parsed.begin());
	if (msg_parsed.size() == 0)
	{
		std::string mode_err = "461 " + user.getNick() + " MODE: need more params";
		send(user.getFd(), mode_err.c_str(), mode_err.size(), 0);
		return 1;
	}
	std::string channelName = msg_parsed[0];
	channelName = channelName.substr(1);
	Channel *targetChannel = findChannelByName(channelName);
	if (checkCmdMode(msg_parsed, user, targetChannel, channelName))
		return 1;
	std::string flag = msg_parsed[0];
	if (flag[1] == 'i')
		targetChannel->modeInvite(flag);
	else if (flag[1] == 'k')
		targetChannel->modePassword(msg_parsed, flag);
	else if (flag[1] == 'l' && targetChannel->modeMaxUsers(msg_parsed, flag, user))
		return 1;
	else if (flag[1] == 'o')
	{
		if (msg_parsed.size() == 1)
		{
			std::cout << RED << channelName << " flag +o: user not inserted" << RESET << std::endl;
			std::string mode_err = "461 " + user.getNick() + " MODE: need more params";
			send(user.getFd(), mode_err.c_str(), mode_err.size(), 0);
			return 1;
		}
		User* new_operator = findUserByNick(msg_parsed[1]);
		if (new_operator == NULL)
		{
			std::string mode_err = user.getNick() + ' ' + msg_parsed[1] + " :No such nick";
			send(user.getFd(), mode_err.c_str(), mode_err.size(), 0);
			return 1;
		}
		if (targetChannel->modeOperator(flag, user, new_operator))
			return 1;
	}
	else if (flag[1] == 't')
		targetChannel->modeTopic(flag);
	return 0;

}

User*	Server::findUserByNick(std::string targetNick)
{
	User* tmp = NULL;
	for (std::vector<User>::iterator it = _users.begin(); it != _users.end(); ++it)
	{
		if (it->getNick() == targetNick)
		{
			*tmp = *it;
			return (tmp);
		}
	}
	return NULL;
}

int		Server::cmdQuit(std::vector<std::string> parsed_message, User &user)
{
	std::string quit_msg = parsed_message.size() >= 2 ? parsed_message[1] : ":Client Quit";
	return (disconnectClient(user.getFd(), quit_msg), 0);
}