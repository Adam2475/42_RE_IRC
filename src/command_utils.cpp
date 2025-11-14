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

int		Server::channelAdder(std::string& channelName, User& user, std::string& pass)
{
	std::vector<Channel>::iterator channelIterator = _channels.begin();
	while (channelIterator != _channels.end())
	{
		std::cout << "searching trough channels" << std::endl;
		if (channelName == channelIterator->getName())
		{
			// TODO rimuovere prints per debug
			std::cout << "channel found" << std::endl;
			if (channelIterator->getInviteOnly())
			{
				if (channelIterator->getInvitedUsersVector().size() > 0)
				{
					std::vector<User>::iterator x = std::find(channelIterator->getInvitedUsersVector().begin(), channelIterator->getInvitedUsersVector().end(), user);
					if (x == channelIterator->getInvitedUsersVector().end())
					{
						std::cout << MAGENTA << x->getNick() << " " << x->getFd() << RESET << std::endl;
						std::string tmp(message_formatter(473, user.getNick(), channelIterator->getName(), "Cannot join channel (+i)"));
						send(user.getFd(), tmp.c_str(), tmp.size(), 0);
						std::cout << RED << user.getNick() << " cannot join channel: invite only restriction" << RESET << std::endl;
						return 1;
					}
					else
						std::cout << BLUE << user.getNick() << " joined channel with restriction" << RESET << std::endl;
				}
				else
				{
					std::cout << MAGENTA << user.getNick() << " no invited users " << user.getFd() << RESET << std::endl;
					std::string tmp(message_formatter(473, user.getNick(), channelIterator->getName(), "Cannot join channel (+i)"));
					send(user.getFd(), tmp.c_str(), tmp.size(), 0);
					std::cout << RED << user.getNick() << " cannot join channel: invite only restriction" << RESET << std::endl;
					return 1;
				}
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