#include "../inc/header.hpp"
#include "../inc/Channel.hpp"
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

int Server::cmdJoin(std::vector<std::string>& mess, User &user)
{
	std::string channelName;
	std::string pass;

	std::cout << "detected command JOIN" << mess[0] << std::endl;
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
	// name, pass, creator, topic, max users, invite_only, topic restriction
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

int		Server::cmdQuit(std::vector<std::string> parsed_message, User &user)
{
	std::string quit_msg;
	std::string out;

	if (parsed_message.size() == 2)
	{
		quit_msg = parsed_message[1];
	}
	else
	{
		quit_msg = ":Client Quit";
	}

	out += ":";
	out += user.getNick();
	out += " QUIT";
	out += quit_msg;
	out += "\r\n";
	//  = ":" + quittingUser.getNickName() + " QUIT " + quit_msg + "\r\n";

    // Erasing the user from all channels
    // for (std::vector<Channel>::reverse_iterator it = _channels.rbegin(); it != _channels.rend(); ++it)
    // {
    //     if (isInVector(quittingUser, it->getUserVector()))
    //     {
    //         //it->writeToChannel(quittingUser, out);
    //         it->partUser(quittingUser, *it, quit_msg);
    //     }
    // }

	disconnectClient(user.getFd());

	return (0);
}