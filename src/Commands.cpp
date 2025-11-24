#include "../inc/header.hpp"
#include "../inc/Server.hpp"
#include "../inc/User.hpp"
#include <set>

int Server::cmdPing(std::vector<std::string> parsed_message, User &user)
{
	if (parsed_message.size() < 2)
	{
		std::string msg = ":server 461 " + user.getNick() + " PING :Not enough parameters\r\n";
		send(user.getFd(), msg.c_str(), msg.size(), 0);
		return (1);
	}
	else
	{
		std::string msg = "PONG " + parsed_message[1] + "\r\n";
		send(user.getFd(), msg.c_str(), msg.size(), 0);
		return (1);
	}
}

int Server::cmdPart(std::vector<std::string> parsed_message, User &user)
{
	std::string channelName;
	std::string reason;

	if (parsed_message.size() < 2)
	{
		std::string msg = ":server 461 " + user.getNick() + " PART :Not enough parameters\r\n";
		send(user.getFd(), msg.c_str(), msg.size(), 0);
		return 1;
	}
	else
		channelName = parsed_message[1];
	if (channelName[0] == '#')
		channelName = channelName.substr(1);
	Channel* targetChannel = findChannelByName(channelName);
	if (targetChannel == NULL) {
		std::string msg = ":server 403 " + user.getNick() + " #" + channelName + " :No such channel\r\n";
		send(user.getFd(), msg.c_str(), msg.size(), 0);
		return 1;
	}
	if (!isInVector(user, targetChannel->getUserVector())) {
		std::string msg = ":server 442 " + user.getNick() + " #" + channelName + " :You're not on that channel\r\n";
		send(user.getFd(), msg.c_str(), msg.size(), 0);
		return 1;
	}
	reason = parsed_message.size() == 3 ? parsed_message[2] : "Leaving";
	targetChannel->partUser(user, reason, PART);
	return 0;
}

int Server::cmdJoin(std::vector<std::string>& mess, User &user)
{
	std::string channelName;
	std::string pass;

	mess.erase(mess.begin());
	channelName = mess[0];
	if (mess[0].empty() || isStrNotPrintable(mess[0].c_str()))
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
	channelCreate(channelName, pass, user);
	return 0;
}

int		Server::cmdPrivateMsg(std::vector<std::string> parsed_message, User &user)
{
	std::string targetsToken;
	std::string msgBody;
	std::string target;
	std::stringstream tss;
	bool is_channel = false;
	size_t i = 0;
	
	if (parsed_message.size() < 3)
	{
		std::string err = ":server 461 " + user.getNick() + " PRIVMSG :Not enough parameters\r\n";
	 	send(user.getFd(), err.c_str(), err.size(), 0);
	 	return (1);		
	}
	else
	{
		targetsToken = parsed_message[1];
		if (parsed_message.size() == 3)
			msgBody = parsed_message[2];
		else if (parsed_message.size() > 3)
		{
			size_t index = 2;
			while (index < parsed_message.size())
			{
				msgBody += parsed_message[index] + ' ';
				index++;
			}
		}
		tss.str(targetsToken);
	}
	while (std::getline(tss, target, ','))
	{
		if (target.empty())
			return (1);
 
		if (target[0] == '#')
		{
			is_channel = true;
			std::string channelName = target.substr(1);
			i = findChannelIndex(channelName);
			if (i == _channels.size())
			{
				std::string err = ":server 403 " + user.getNick() + " " + target + " :No such channel\r\n";
				send(user.getFd(), err.c_str(), err.size(), 0);
				return (1);
			}
			else
			{
				if (!isInVector(user, _channels[i].getUserVector()))
				{
					std::string err = ":server 404 " + user.getNick() + " " + target + " :Cannot send to channel\r\n";
					send(user.getFd(), err.c_str(), err.size(), 0);
					continue;
				}
			}
		}
		else
		{
			i = findUserIndex(target);
			if (i == _users.size())
			{
				std::string err = ":server 401 " + user.getNick() + " " + target + " :There was no such nickname\r\n";
				send(user.getFd(), err.c_str(), err.size(), 0);
				return (1);
			}
		}
		std::string out = ":" + user.getNick() + " PRIVMSG " + target + " :" + msgBody + "\r\n";
		is_channel ? _channels[i].writeToChannel(out, user.getNick()) : (void)send(_users[i].getFd(), out.c_str(), out.size(), 0);
	}
	return (0);
}

int Server::cmdWho(std::vector<std::string> parsed_message, User &user)
{
    std::string target = "";
    if (parsed_message.size() > 1)
        target = parsed_message[1];
    if (!target.empty() && target[0] == '#')
    {
        target = target.substr(1);
        Channel *ch = findChannelByName(target);
        if (!ch)
        {
            std::string msg = ":server 403 " + user.getNick() + " #" + target + " :No such channel\r\n";
            send(user.getFd(), msg.c_str(), msg.size(), 0);
            return 0;
        }
        std::vector<User> channelUsers = ch->getUserVector();
        for (size_t i = 0; i < channelUsers.size(); ++i)
        {
            std::string msg = ":server 352 " + user.getNick() + " #" + target + " " + channelUsers[i].getUser()
				+ " localhost " + "server " + channelUsers[i].getNick() + " H :0 " + channelUsers[i].getNick() + "\r\n";
            send(user.getFd(), msg.c_str(), msg.size(), 0);
        }
    }
    else
    {
        for (size_t i = 0; i < _users.size(); ++i)
        {
            std::string msg = ":server 352 " + user.getNick() + " * " + _users[i].getUser() + " localhost "
				+ "server " + _users[i].getNick() + " H :0 " + _users[i].getNick() + "\r\n";
            send(user.getFd(), msg.c_str(), msg.size(), 0);
        }
    }
    std::string end_msg = ":server 315 " + user.getNick() + " " + (target.empty() ? "*"
		: target) + " :End of WHO list\r\n";
    send(user.getFd(), end_msg.c_str(), end_msg.size(), 0);
    return 0;
}

int		Server::cmdInvite(std::vector<std::string> parsed_message, User &user)
{
	std::string targetNick;
	std::string channelName;

	if (parsed_message.size() < 3)
	{
		std::string err = ":server 461 " + user.getNick() + " INVITE :Not enough parameters\r\n";
	 	send(user.getFd(), err.c_str(), err.size(), 0);
	 	return (1);				
	}
	targetNick = parsed_message[1];
	channelName = parsed_message[2];
	if (targetNick.empty() || channelName.empty())
	{
		std::string tmp(message_formatter2(461, "INVITE", ":Not enough parameters\r\n"));
		send(user.getFd(), tmp.c_str(), tmp.size(), 0);
		return (1);
	}
	User* targetUser = findUserByNick(targetNick);
	if (targetUser == NULL)
	{
		std::string tmp = ":server 401 " + targetNick + " :There was no such nickname\r\n";
		send(user.getFd(), tmp.c_str(), tmp.size(), 0);
		return 1;
	}
	if (channelName[0] == '#')
		channelName = channelName.substr(1);
	Channel *targetChannel = findChannelByName(channelName);
	if (targetUser->getNick().empty())
	{
		std::string err = ":server 401 " + user.getNick() + " " + targetNick + " :There was no such nickname\r\n";
		send(user.getFd(), err.c_str(), err.size(), 0);
		return (1);
	}
	if (targetChannel == NULL)
	{
		std::string err = ":server 403 " + user.getNick() + " " + channelName + " :No such channel\r\n";
		send(user.getFd(), err.c_str(), err.size(), 0);
		return (1);
	}
	if (!isInVector(user, targetChannel->getUserVector())) {
		std::string err = ":server 442 " + user.getNick() + " #" + channelName + " :You're not on that channel\r\n";
		send(user.getFd(), err.c_str(), err.size(), 0);
		return 1;
	}
	if (isInVector(*targetUser, targetChannel->getUserVector())) {
		std::string err = ":server 443 " + user.getNick() + " " + targetNick + " #" + channelName + " :is already on channel\r\n";
		send(user.getFd(), err.c_str(), err.size(), 0);
		return 1;
	}
	targetChannel->addToInvited(*targetUser);
	std::string inviting_msg = ":server 341 " + user.getNick() + " " + targetNick + " #" + channelName + "\r\n";
	send(user.getFd(), inviting_msg.c_str(), inviting_msg.size(), 0);
	std::string invite_msg = ":" + user.getNick() + " INVITE " + targetNick + " :#" + channelName + "\r\n";
	send(targetUser->getFd(), invite_msg.c_str(), invite_msg.size(), 0);
	return (0);
}

int		Server::cmdTopic(std::vector<std::string> parsed_message, User &user)
{
	std::string channel_name;
	std::string arg2;

	if (parsed_message.size() < 2)
	{
		std::string err_msg = message_formatter2(461, "TOPIC", "need more params");
		send(user.getFd(), err_msg.c_str(), err_msg.size(), 0);
		return 1;
	}
	channel_name = parsed_message[1];
	if (channel_name.empty())
	{
		std::string err_msg = message_formatter2(461, "TOPIC", "need more params");
		send(user.getFd(), err_msg.c_str(), err_msg.size(), 0);
		return 1;
	}
	if (removeInitialHash(&channel_name))
	{
		std::string err_msg = message_formatter2(461, "TOPIC", "need more params");
		send(user.getFd(), err_msg.c_str(), err_msg.size(), 0);
		return 1;
	}
	Channel *targetChannel = findChannelByName(channel_name);
	if (!targetChannel)
	{
		std::string err_msg = message_formatter2(403, "TOPIC", "no such channel");
		send(user.getFd(), err_msg.c_str(), err_msg.size(), 0);
		return 1;
	}
	if (!isInVector(user, targetChannel->getUserVector()))
	{
		std::string err_msg = message_formatter(442, user.getNick(), channel_name, "You're not on that channel");
		send(user.getFd(), err_msg.c_str(), err_msg.size(), 0);
		return 1;
	}
	if (parsed_message.size() > 2)
	{
		if (targetChannel->getTopicRestriction() && !isInVector(user, targetChannel->getUserOperatorsVector()))
		{
			std::string err_msg = message_formatter(482, user.getNick(), channel_name, "You're not a channel operator");
			send(user.getFd(), err_msg.c_str(), err_msg.size(), 0);
			return 1;
		}
		size_t index = 2;
		while (index < parsed_message.size())
		{
			if (index + 1 < parsed_message.size())
				arg2 += parsed_message[index] + ' ';
			else
				arg2 += parsed_message[index];
			index++;
		}
		targetChannel->setTopic(arg2);
		std::string broadcast = ":" + user.getNick() + "!" + user.getUser() + "@host TOPIC #" + channel_name + " :" + arg2 + "\r\n";
		targetChannel->writeToChannel(broadcast, user.getNick());
		targetChannel->showChannelTopic(user, "server");
		return 0;
	}
	else
		targetChannel->showChannelTopic(user, "server");
	return (0);
}

int	Server::checkCmdMode(std::vector<std::string>& msg_parsed, User& user, Channel* targetChannel, std::string& channelName)
{
	if (targetChannel == NULL)
	{
		std::string mode_err = message_formatter(403, user.getNick(), channelName, "No such channel");
		send(user.getFd(), mode_err.c_str(), mode_err.size(), 0);
		return 1;
	}
	if (!isInVector(user, targetChannel->getUserVector()))
	{
		std::string mode_err = message_formatter(442, user.getNick(), channelName, "You're not on channel");
		send(user.getFd(), mode_err.c_str(), mode_err.size(), 0);
		return 1;
	}
	msg_parsed.erase(msg_parsed.begin());
	if (msg_parsed.size() == 0)
	{
		std::string modes;
		if (targetChannel->getTopicRestriction())
			modes += 't';
		if (!targetChannel->getPassword().empty())
			modes += 'k';
		if (targetChannel->getMaxUsers())
			modes += 'l';
		if (targetChannel->getInviteOnly())
			modes += 'i';
		std::string reply = ":server 324 " + user.getNick() +
							" #" + channelName + " +" + modes;
		if (!targetChannel->getPassword().empty())
			reply += " " + targetChannel->getPassword();
		if (targetChannel->getMaxUsers() > 0)
		{
			std::stringstream ss;
			ss << targetChannel->getMaxUsers();
			reply += " " + ss.str();
		}
		reply += "\r\n";
		send(user.getFd(), reply.c_str(), reply.size(), 0);
		return 1;
	}
	if (!isInVector(user, targetChannel->getUserOperatorsVector()) && msg_parsed[0] != "+b")
	{
		std::string mode_err = message_formatter(482, user.getNick(), channelName, "You're not channel operator");
		send(user.getFd(), mode_err.c_str(), mode_err.size(), 0);
		return 1;
	}
	return 0;
}

bool		Server::flagProcessing(std::string flag, Channel& targetChannel, User& user, std::vector<std::string>& msg_parsed)
{
	if (flag[1] == 'i')
	{
		targetChannel.modeInvite(flag, user);
		return 0;
	}
	else if (flag[1] == 'k')
	{
		targetChannel.modePassword(msg_parsed, flag, user);
		return 0;
	}
	else if (flag[1] == 'l')
	{
		if (targetChannel.modeMaxUsers(msg_parsed, flag, user))
			return 1;
		return 0;
	}
	else if (flag[1] == 'o')
	{
		if (msg_parsed.size() == 1)
		{
			std::string mode_err = "461 " + user.getNick() + " MODE: need more params\r\n";
			send(user.getFd(), mode_err.c_str(), mode_err.size(), 0);
			return 1;
		}
		User* new_operator = NULL;
		size_t i = 1;
		for (i = 1; i < msg_parsed.size(); i++)
		{
			User* new_operator = findUserByNick(msg_parsed[i]);
			if (new_operator)
				break;
		}
		if (new_operator == NULL)
		{
			std::string mode_err = user.getNick() + ' ' + msg_parsed[1] + " :No such nick\r\n";
			send(user.getFd(), mode_err.c_str(), mode_err.size(), 0);
			return 1;
		}
		msg_parsed.erase(msg_parsed.begin() + i);
		if (targetChannel.modeOperator(flag, user, new_operator))
			return 1;
		return 0;
	}
	else if (flag[1] == 't')
	{
		targetChannel.modeTopic(flag, user);
		return 0;
	}
	else if (flag[1] == 'b')
		return 0;
	else
	{
		std::string mode_err = "501 " + user.getNick() + " :Unknown MODE flag\r\n";
		send(user.getFd(), mode_err.c_str(), mode_err.size(), 0);
		return 1;
	}
	return 0;
}

int Server::cmdMode(std::vector<std::string>& msg_parsed, User& user)
{
	msg_parsed.erase(msg_parsed.begin());
	if (msg_parsed.size() == 0)
	{
		std::string mode_err = "461 " + user.getNick() + " MODE: need more params\r\n";
		send(user.getFd(), mode_err.c_str(), mode_err.size(), 0);
		return 1;
	}
	std::string channelName = msg_parsed[0];
	channelName = channelName.substr(1);
	Channel *targetChannel = findChannelByName(channelName);
	if (checkCmdMode(msg_parsed, user, targetChannel, channelName))
		return 1;
	if (msg_parsed.size() == 0)
		return 0;
	std::string flag = msg_parsed[0];
	std::string tmp_flag("  ");
	size_t i = 0;
	size_t j = 0;

	while (j < flag.size())
	{
		if (flag[j] == '-'|| flag[j] == '+')
		{
			tmp_flag[0] = flag[j];
			i = j + 1;
			while (i < flag.size() && flag[i] != '-' && flag[i] != '+')
			{
				tmp_flag[1] = flag[i];
				flagProcessing(tmp_flag, *targetChannel, user, msg_parsed);
				i++;
			}
		}
		j = i;
	}
	return 0;
}

int		Server::cmdQuit(std::vector<std::string> parsed_message, User &user)
{
	std::string quit_msg = parsed_message.size() >= 2 ? parsed_message[1] : ":Client Quit";
	return (disconnectClient(user.getFd(), quit_msg), 0);
}

int		Server::cmdKick(std::vector<std::string> parsed_message, User &user)
{
	std::string channelName;

	Channel *targetChannel = NULL;
	if (parsed_message.size() < 3)
	{	
		std::string kick_err = message_formatter2(461, "KICK", "need more params");
		send(user.getFd(), kick_err.c_str(), kick_err.size(), 0);
		return 1;
	}
	if (!parsed_message[1].empty())
	{
		channelName = parsed_message[1].substr(1);
		targetChannel = findChannelByName(channelName);
	}
	if (!targetChannel)
	{
		std::string kick_err = message_formatter2(403, "KICK", "no such channel");
		send(user.getFd(), kick_err.c_str(), kick_err.size(), 0);
		return 1;
	}
	if (!isInVector(user, targetChannel->getUserOperatorsVector()))
	{
		std::string kick_err = message_formatter(482, user.getNick(), channelName, "You're not an operator");
		send(user.getFd(), kick_err.c_str(), kick_err.size(), 0);
		return 1;
	}
	User* kicked_user = findUserByNick(parsed_message[2]);
	if (!kicked_user)
	{
		std::string kick_err = ":server 401 " + user.getNick() + ' ' + parsed_message[2] + " :No such nick\r\n";
		send(user.getFd(), kick_err.c_str(), kick_err.size(), 0);
		return 1;
	}
	if (!isInVector(*kicked_user, targetChannel->getUserVector()))
	{
		std::string kick_err = ":server 441 "
			+ user.getNick() + ' '
			+ kicked_user->getNick() + ' '
			+ channelName + " :They aren't on that channel\r\n";
		send(user.getFd(), kick_err.c_str(), kick_err.size(), 0);
		return 1;
	}
	std::stringstream reason;
	if (parsed_message.size() >= 4)
	{
		size_t i = 2;
		while (++i < parsed_message.size())
		{
			if (i == 3)
				reason << parsed_message[i];
			else
				reason << ' ' << parsed_message[i];
		}
	}
	targetChannel->kickUser(*kicked_user, user, reason.str());
	return 0;
}

int Server::cmdUser(std::vector<std::string> parsed_message, User *sending_user)
{
	if (parsed_message.size() != 5)
	{
		std::string message = ":server 461 ";
		message += sending_user->getNick().empty() ? "*" : sending_user->getNick();
		message += " :USER :Not enough parameters\n\r";
		send(sending_user->getFd(), message.c_str(), message.size(), 0);
		return (1);
	}
	else
	{
		if (isValidNick(parsed_message[1]))
			sending_user->setUser(parsed_message[1]);
		else
		{
			std::string message = ":server 461 ";
			message += sending_user->getNick().empty() ? "*" : sending_user->getNick();
			message += " :Erroneous Username\n\r";
			send(sending_user->getFd(), message.c_str(), message.size(), 0);
			return (1);
		}	
	}
	return (0);
}

int		Server::cmdNick(std::vector<std::string> parsed_message, User *sending_user)
{
	if (parsed_message.size() < 2 || parsed_message[1].empty())
	{
		std::string message;
		message += ":server 461 ";
		message += sending_user->getNick().empty() ? "*" : sending_user->getNick();
		message += " :Not enough parameters\n\r";
		send(sending_user->getFd(), message.c_str(), message.size(), 0);
		return (1);
	}
	if (check_existing_user(_users, parsed_message[1]))
	{
		std::string message;
		message += ":server 433 ";
		message += sending_user->getNick().empty() ? "*" : sending_user->getNick();
		message += " :Nickname is already in use\n\r";
		send(sending_user->getFd(), message.c_str(), message.size(), 0);
		return (1);
	}
	else
	{
		if (isValidNick(parsed_message[1]))
		{
			std::string oldNick = sending_user->getNick();
			std::string newNick = parsed_message[1];

			sending_user->setNick(newNick);
			if (!oldNick.empty())
			{
				for (size_t i = 0; i < _channels.size(); ++i)
					_channels[i].updateUserNickByFd(sending_user->getFd(), newNick);
				std::string prefixUser = sending_user->getUser();
				std::string broadcast = ":" + oldNick + "!" + prefixUser + "@host NICK " + newNick + "\r\n";
				std::set<int> recipients;
				recipients.insert(sending_user->getFd());
				for (size_t ci = 0; ci < _channels.size(); ++ci)
				{
					std::vector<User> chUsers = _channels[ci].getUserVector();
					bool inChannel = false;
					for (size_t uj = 0; uj < chUsers.size(); ++uj)
					{
						if (chUsers[uj].getFd() == sending_user->getFd())
						{
							inChannel = true;
							break;
						}
					}
					if (inChannel)
					{
						for (size_t uj = 0; uj < chUsers.size(); ++uj)
							recipients.insert(chUsers[uj].getFd());
					}
				}
				for (std::set<int>::iterator it = recipients.begin(); it != recipients.end(); ++it)
					send(*it, broadcast.c_str(), broadcast.size(), 0);
			}
		}
		else
		{
			std::string message;
			message += ":server 432 ";
			message += sending_user->getNick().empty() ? "*" : sending_user->getNick();
			message += " :Erroneous Nickname\n\r";
			send(sending_user->getFd(), message.c_str(), message.size(), 0);
			return (1);
		}
	}
	return (0);
}