#include "../inc/header.hpp"
#include "../inc/Server.hpp"
#include "../inc/User.hpp"

int Server::cmdPing(std::vector<std::string> parsed_message, User &user)
{
	//std::cout << "PING received" << std::endl;
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

	reason = parsed_message.size() == 3 ? parsed_message[2] : "Leaving";
	targetChannel->partUser(user, *targetChannel, reason, PART);

	return 0;
}

int Server::cmdJoin(std::vector<std::string>& mess, User &user)
{
	std::string channelName;
	std::string pass;

	// std::cout << "detected command JOIN" << mess[0] << std::endl;
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
	//std::cout << channelName << " channel not found, creating..." << std::endl;
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
					// ERR_CANNOTSENDTOCHAN (404)
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
            std::string msg = ":server 403 ";
            msg += user.getNick();
            msg += " #";
            msg += target;
            msg += " :No such channel\r\n";
            send(user.getFd(), msg.c_str(), msg.size(), 0);
            return 0;
        }
        // list users in channel
        std::vector<User> channelUsers = ch->getUserVector();
        for (size_t i = 0; i < channelUsers.size(); ++i)
        {
            std::string msg = ":server 352 ";
            msg += user.getNick();
            msg += " #";
            msg += target;
            msg += " ";
            msg += channelUsers[i].getUser();
            msg += " localhost ";
            msg += "server ";
            msg += channelUsers[i].getNick();
            msg += " H :0 ";
            msg += channelUsers[i].getNick();
            msg += "\r\n";
            send(user.getFd(), msg.c_str(), msg.size(), 0);
        }
    }
    else
    {
        // list all users on server
        for (size_t i = 0; i < _users.size(); ++i)
        {
            std::string msg = ":server 352 ";
            msg += user.getNick();
            msg += " * ";
            msg += _users[i].getUser();
            msg += " localhost ";
            msg += "server ";
            msg += _users[i].getNick();
            msg += " H :0 ";
            msg += _users[i].getNick();
            msg += "\r\n";
            send(user.getFd(), msg.c_str(), msg.size(), 0);
        }
    }

    // send end-of-list
    std::string end_msg = ":server 315 ";
    end_msg += user.getNick();
    end_msg += " ";
    end_msg += (target.empty() ? "*" : target);
    end_msg += " :End of WHO list\r\n";
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

	// TODO mancano errori e sending

	if (targetNick.empty() || channelName.empty())
	{
		// ERR_NEEDMOREPARAMS (461)
		std::string tmp(message_formatter2(461, "INVITE", ":Not enough parameters\r\n"));
		send(user.getFd(), tmp.c_str(), tmp.size(), 0);
		std::cout << "wrong number of arguments" << std::endl;
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
	{
		channelName = channelName.substr(1);
	}
	Channel *targetChannel = findChannelByName(channelName);

	
	if (targetUser->getNick().empty())
	{
		std::string err = ":server 401 " + user.getNick() + " " + targetNick + " :There was no such nickname\r\n";
		send(user.getFd(), err.c_str(), err.size(), 0);
		return (1);
	}

	std::cout << "found user: " << targetUser->getNick() << std::endl;
	std::cout << channelName << std::endl;

	if (targetChannel == NULL)
	{
		std::string err = ":server 403 " + user.getNick() + " " + channelName + " :No such channel\r\n";
		send(user.getFd(), err.c_str(), err.size(), 0);
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
	if (isInVector(*targetUser, targetChannel->getUserVector())) {
		std::string err = ":server 443 " + user.getNick() + " " + targetNick + " #" + channelName + " :is already on channel\r\n";
		send(user.getFd(), err.c_str(), err.size(), 0);
		return 1;
	}

	// send invite
	targetChannel->addToInvited(*targetUser);

	// Send RPL_INVITING to user
	std::string inviting_msg = ":server 341 " + user.getNick() + " " + targetNick + " #" + channelName + "\r\n";
	send(user.getFd(), inviting_msg.c_str(), inviting_msg.size(), 0);

	// Send INVITE to target user
	std::string invite_msg = ":" + user.getNick() + " INVITE " + targetNick + " :#" + channelName + "\r\n";
	send(targetUser->getFd(), invite_msg.c_str(), invite_msg.size(), 0);


	return (0);
}

int		Server::cmdTopic(std::vector<std::string> parsed_message, User &user)
{
	std::string channel_name;
	std::cout << "detected command TOPIC" << std::endl;
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
	std::cout << channel_name << std::endl;

	if (removeInitialHash(&channel_name))
	{
		std::string err_msg = message_formatter2(461, "TOPIC", "need more params");
		send(user.getFd(), err_msg.c_str(), err_msg.size(), 0);
		return 1;
	}
	else
	{
		std::cout << "hash removed correctly" << std::endl;
		std::cout << channel_name << std::endl;
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
		return 0;
	}
	else
		targetChannel->showChannelTopic(user, "server");
	return (0);
}

void print_vec(std::vector<User>& parsed_message)
{
	for (size_t i = 0; i < parsed_message.size(); i++)
	{
		std::cout << parsed_message[i].getNick() << ' ' << parsed_message[i].getFd() << ' ' << i << std::endl;
	}
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
		return 0;
	}
	if (!isInVector(user, targetChannel->getUserOperatorsVector()))
	{
		std::vector<User> vect = targetChannel->getUserOperatorsVector();
		print_vec(vect);
		std::cout << RED << user.getNick() << " not an operator" << RESET << std::endl;
		std::string mode_err = message_formatter(482, user.getNick(), channelName, "You're not channel operator");
		send(user.getFd(), mode_err.c_str(), mode_err.size(), 0);
		return 1;
	}
	std::string flag = msg_parsed[0];
	std::string mode[11] = {"+b", "+i", "-i", "+k", "-k", "+o", "-o", "+l", "-l", "+t", "-t"};
	if (std::find(mode, mode+11, flag) == mode + 11)
	{
		std::cout << RED << channelName << " unknown flag " << flag << RESET << std::endl;
		std::string mode_err = "501 " + user.getNick() + " :Unknown MODE flag\r\n";
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
			std::string mode_err = "461 " + user.getNick() + " MODE: need more params\r\n";
			send(user.getFd(), mode_err.c_str(), mode_err.size(), 0);
			return 1;
		}
		User* new_operator = findUserByNick(msg_parsed[1]);
		if (new_operator == NULL)
		{
			std::string mode_err = user.getNick() + ' ' + msg_parsed[1] + " :No such nick\r\n";
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

int		Server::cmdQuit(std::vector<std::string> parsed_message, User &user)
{
	std::string quit_msg = parsed_message.size() >= 2 ? parsed_message[1] : ":Client Quit";
	return (disconnectClient(user.getFd(), quit_msg), 0);
}

int		Server::cmdKick(std::vector<std::string> parsed_message, User &user)
{
	std::cout << "find command KICK" << std::endl;
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
	std::vector<User>::iterator it = std::find(
			targetChannel->getUserVector().begin(),
			targetChannel->getUserVector().end(),
			*kicked_user
		);
	if (it == targetChannel->getUserVector().end())
	{
		std::cout << RED << "didn't found the user" << RESET << std::endl;
		std::string kick_err = ":server 441 "
			+ user.getNick() + ' '
			+ kicked_user->getNick() + ' '
			+ channelName + " :They aren't on that channel\r\n";
		send(user.getFd(), kick_err.c_str(), kick_err.size(), 0);
		return 1;
	}
	if (isInVector(*kicked_user, targetChannel->getUserOperatorsVector()))
	{
		std::vector<User>::iterator op_it = std::find(
				targetChannel->getUserOperatorsVector().begin(),
				targetChannel->getUserOperatorsVector().end(),
				*kicked_user
			);
		targetChannel->getUserOperatorsVector().erase(op_it);
	}
	targetChannel->getUserVector().erase(it);
	return 0;
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
			sending_user->setNick(parsed_message[1]);
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