#include "../inc/header.hpp"
// #include "../inc/Channel.hpp"
#include "../inc/Server.hpp"

User*	Server::findUserByNick(std::string& targetNick)
{
	for (std::vector<User>::iterator it = _users.begin(); it != _users.end(); ++it)
	{
		if (it->getNick() == targetNick)
		{
			return &(*it);
		}
	}
	return NULL;
}

size_t Server::findChannelIndex(std::string target)
{
	size_t i = 0;
	while (i < _channels.size())
	{
		if (_channels[i].getName() == target)
			break;
		i++;
	}
	return (i);
}

size_t Server::findUserIndex(std::string target)
{
	size_t i = 0;
	while (i < _users.size())
	{
		if (_users[i].getNick() == target)
			break;
		i++;
	}	
	return (i);
}

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
	Channel new_channel(channelName, pass, user, topic, -1, 0, 1);
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

void		Server::join_message_confirm(User &user, Channel& channel) const
{
	// Standard IRC Replies for JOIN
	std::string join_msg = ":" + user.getNick() + " JOIN #" + channel.getName() + "\r\n";
	channel.writeToChannel(join_msg);
	std::string topic_msg = ":server 332 " + user.getNick() + " #" + channel.getName() + " :" + channel.getTopic() + "\r\n";
	send(user.getFd(), topic_msg.c_str(), topic_msg.size(), 0);
	std::string users_list = channel.getNickList();
	std::string namreply_msg = ":server 353 " + user.getNick() + " = #" + channel.getName() + " :" + users_list + "\r\n";
	channel.writeToChannel(namreply_msg);
	std::string endofnames_msg = ":server 366 " + user.getNick() + " #" + channel.getName() + " :End of /NAMES list.\r\n";
	channel.writeToChannel(endofnames_msg);
}


void print_vec(std::vector<User>& parsed_message)
{
	for (size_t i = 0; i < parsed_message.size(); i++)
	{
		std::cout << parsed_message[i].getNick() << ' ' << parsed_message[i].getFd() << ' ' << i << std::endl;
	}
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
					size_t i = 0;
					std::vector<User> invited_vect = channelIterator->getInvitedUsersVector();
					print_vec(invited_vect);
					while (i < invited_vect.size())
					{
						std::cout << YELLOW << invited_vect[i].getNick() << RESET << std::endl;
						if (invited_vect[i] == user)
						{
							channelIterator->addUserToChannel(user, pass);
							join_message_confirm(user, *channelIterator);
							return (0);
						}
						i++;
					}
					if (invited_vect.begin() + i == invited_vect.end())
					{
						std::string tmp(message_formatter(473, user.getNick(), channelIterator->getName(), "Cannot join channel (+i)"));
						send(user.getFd(), tmp.c_str(), tmp.size(), 0);
						std::cout << RED << user.getNick() << " cannot join channel: invite only restriction" << RESET << std::endl;
						return 1;
					}
				}
				else
				{
					std::cout << MAGENTA << user.getNick() << " :No invited users are present" << RESET << std::endl;
					std::string tmp(message_formatter(473, user.getNick(), channelIterator->getName(), "Cannot join channel (+i)"));
					send(user.getFd(), tmp.c_str(), tmp.size(), 0);
					std::cout << RED << user.getNick() << " cannot join channel: invite only restriction" << RESET << std::endl;
					return 1;
				}
			}
			channelIterator->addUserToChannel(user, pass);
			join_message_confirm(user, *channelIterator);			
			return (0);
		}
		++channelIterator;
	}
	return -1;
}