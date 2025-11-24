#include "../inc/Channel.hpp"

//////////////////
// Constructors
//////////////////

Channel::Channel() {
	_max_users = -1;
	_topic_restriction = true;
	_invite_only = false;
}

Channel::Channel(const Channel& other) : _user_vector(other._user_vector), 
_operators_vector(other._operators_vector), _name(other._name),
_passwd(other._passwd), _topic(other._topic), _max_users(other._max_users), _invited_users(other._invited_users),
_invite_only(other._invite_only), _topic_restriction(other._topic_restriction) {}

Channel& Channel::operator=(const Channel& other)
{
	if (this != &other)
	{
		_name = other._name;
		_user_vector = other._user_vector;
		_operators_vector = other._operators_vector;
		_invited_users = other._invited_users;
		_passwd = other._passwd;
		_topic = other._topic;
		_max_users = other._max_users;
		_topic_restriction = other._topic_restriction;
		_invite_only = other._invite_only;
	}
	return *this;
}

Channel::Channel(std::string& name, std::string& passwd, User& creator, std::string& topic
	, size_t max_users, bool invite_only, bool topic_restriction) : 
	_name(name), _passwd(passwd), _topic(topic), _max_users(max_users)
	, _invite_only(invite_only), _topic_restriction(topic_restriction)
{
	_user_vector.push_back(creator);
	_operators_vector.push_back(creator);
}

Channel::~Channel()
{}

/////////////
// Getters
/////////////

std::string	Channel::getPassword() const
{
	return _passwd;
}

std::vector<User> Channel::getUserVector() const
{
	std::vector<User> new_vect(_user_vector);
	return new_vect;
}

std::vector<User> Channel::getUserOperatorsVector() const
{
	std::vector<User> new_vect(_operators_vector);
	return new_vect;
}

std::string Channel::getName() const
{
	return _name;
}

bool 	Channel::getInviteOnly() const
{
	return _invite_only;
}

std::string	Channel::getTopic() const
{
	return _topic;
}

std::vector<User> Channel::getInvitedUsersVector() const
{
	return _invited_users;
}

size_t  Channel::getMaxUsers() const
{
	return _max_users;
}

bool    Channel::getTopicRestriction() const
{
	return _topic_restriction;
}

std::string Channel::getNickList() const
{
	std::string users_list;
	for (size_t i = 0; i < _user_vector.size(); i++)
	{
		if (isInVector(const_cast<User&>(_user_vector[i]), this->getUserOperatorsVector()))
			users_list += '@' + _user_vector[i].getNick() + " ";
		else
			users_list += _user_vector[i].getNick() + " ";
	}
	return users_list;
}

/////////////
// Setters
/////////////

void	Channel::setTopicRestriction(bool set)
{
	_topic_restriction = set;
}

void	Channel::setName(std::string& name)
{
	_name = name;
}

void	Channel::setTopic(std::string& topic)
{
	_topic = topic;  
}

void	Channel::setPassword(std::string &pass)
{
	_passwd = pass;
}

void	Channel::setMaxUsers(size_t num)
{
	_max_users = num;
}

void	Channel::setInviteOnly(bool set)
{
	_invite_only = set;
}

//////////////////////
// Public Functions
//////////////////////

void Channel::addToInvited(User& user)
{
    if (!isInVector(user, _invited_users))
    {
        _invited_users.push_back(user);
    }
}

bool	Channel::isOperatorUser(User target_user) const
{
	return (isInVector(target_user, _operators_vector) ? true : false);
}

void Channel::showChannelTopic(User &user, const std::string serverName)
{
    const std::string &topic = getTopic();
    std::string reply;

	if (topic.empty())
	{
		reply = ":" + serverName 
			  + " 331 " + user.getNick() 
			  + " #" + _name 
			  + " :No topic is set\r\n";
	}
	else
	{
		reply = ":" + serverName 
			  + " 332 " + user.getNick() 
			  + " #" + _name 
			  + " :" + topic + "\r\n";
	}

	send(user.getFd(), reply.c_str(), reply.size(), MSG_NOSIGNAL);
}

void	Channel::kickUser(User& user, User& user_operator, std::string reason)
{
	std::string reply_kick;
	if (reason.empty())
		reply_kick = ":" + user_operator.getNick() + "!" + user.getUser() + 
			"@localhost KICK #" + this->_name + ' ' + user.getNick() + " :" + user_operator.getNick() + "\r\n";
	else
		reply_kick = ":" + user_operator.getNick() + "!" + user.getUser() + 
			"@localhost KICK #" + this->_name + ' ' + user.getNick() + " :" + reason + "\r\n";
	this->writeToChannel(reply_kick, "");
	std::vector<User>::iterator it;
	for (it = _user_vector.begin(); it != _user_vector.end(); ++it)
	{
		if (*it == user)
		{
			_user_vector.erase(it);
			break;
		}
	}
	if (isInVector(user, _operators_vector))
	{
		for(it = _operators_vector.begin(); it != _operators_vector.end(); ++it)
		{
			if (*it == user)
			{
				_operators_vector.erase(it);
				break;
			}
		}
	}
}

void	Channel::partUser(User& user, std::string msg, int mode)
{
	std::string user_prefix = user.getNick() + "!" + user.getUser() + "@";
	std::string part_msg;
	std::string userNick = user.getNick();
	std::string userUser = user.getNick();

	for (std::vector<User>::iterator it = _user_vector.begin(); it != _user_vector.end(); ++it)
	{
		if (user == *it)
		{
			_user_vector.erase(it);
			break;
		}
	}
	if (isInVector(user, _operators_vector))
	{
		for (std::vector<User>::iterator it = _operators_vector.begin(); it != _operators_vector.end(); ++it)
		{
			if (user == *it)
			{
				_operators_vector.erase(it);
				break;
			}
		}
	}
	
	if (mode == PART)
		part_msg = ":" + user_prefix + " PART #" + _name + " :" + msg + "\r\n";
	else if (mode == QUIT)
		part_msg = ":" + user_prefix + " QUIT" + " :" + msg + "\r\n";
	this->writeToChannel(part_msg, user.getNick());
	send(user.getFd(), part_msg.c_str(), part_msg.size(), MSG_NOSIGNAL);
	if (_user_vector.size() == 0)
	{
		_name.erase(0, _name.size());
		return ;
	}
	if (_operators_vector.size() == 0
		&& _user_vector.size() > 0)
	{
		_operators_vector.push_back(*_user_vector.begin());
		std::string msg = ":" + userNick + "!" + userUser + "@host MODE #" + _name + " +o " + _user_vector.begin()->getNick() + "\r\n";
		this->writeToChannel(msg, user.getNick());
	}
}

int	Channel::addUserToChannel(User& user, std::string& passwd)
{
	if (!_passwd.empty() && _passwd.compare(passwd) != 0)
	{
		std::string tmp(message_formatter(475, user.getNick(), _name, "Cannot join channel (+k)"));
		send(user.getFd(), tmp.c_str(), tmp.size(), MSG_NOSIGNAL);
		return 1;
	}
	if (isInVector(user, _user_vector))
	{
		std::string tmp(message_formatter(443, user.getNick(), _name, "is already on channel"));
		send(user.getFd(), tmp.c_str(), tmp.size(), MSG_NOSIGNAL);
		return 1;
	}
	if (_user_vector.size() == _max_users)
	{
		std::string tmp(message_formatter(471, user.getNick(), _name, "Channel is full"));
		send(user.getFd(), tmp.c_str(), tmp.size(), MSG_NOSIGNAL);
		return 1;
	}
	else
		_user_vector.push_back(user);
	return 0;
}


void	Channel::addUserToOperatorsVector(User& user, User& user_operator)
{
	if (!isInVector(user_operator, _operators_vector))
	{
		std::string tmp(message_formatter(481, user_operator.getNick(), _name, "You're not an operator"));
		send(user_operator.getFd(), tmp.c_str(), tmp.size(), MSG_NOSIGNAL);
		return ;
	}
	else if (!isInVector(user, _user_vector))
	{
		std::string tmp(message_formatter(442, user.getNick(), _name, "You are not part of the channel"));
		send(user.getFd(), tmp.c_str(), tmp.size(), MSG_NOSIGNAL);
		return ;
	}
	else
		_operators_vector.push_back(user);
}


void Channel::writeToChannel(std::string& buffer, std::string sending_nick) const
{
	for (std::vector<User>::const_iterator it = _user_vector.begin(); it != _user_vector.end(); ++it)
	{
		if (it->getNick() != sending_nick)
			send(it->getFd(), buffer.c_str(), buffer.size(), MSG_NOSIGNAL);
	}
}

void Channel::updateUserNickByFd(int fd, const std::string& newNick)
{
	for (size_t i = 0; i < _user_vector.size(); ++i)
	{
		if (_user_vector[i].getFd() == fd)
			_user_vector[i].setNick(newNick);
	}
	for (size_t i = 0; i < _operators_vector.size(); ++i)
	{
		if (_operators_vector[i].getFd() == fd)
			_operators_vector[i].setNick(newNick);
	}
}

// Mode commands


std::string mode_msg_formatter(User& user, std::string mode, std::string& channelName)
{
	std::string msg = ":" + user.getNick() + "!" + user.getUser() + "@host MODE #" + channelName + ' ' + mode + "\r\n";
	return msg;
}

void	Channel::modeInvite(std::string& arg, User& user)
{	
	if (arg[0] == '+')
		setInviteOnly(true);
	else if (arg[0] == '-')
		setInviteOnly(false);
	std::string reply = mode_msg_formatter(user, arg, this->_name);
	this->writeToChannel(reply, "");
}

void	Channel::modePassword(std::vector<std::string>& msg_parsed, std::string& arg, User& user)
{
	if (arg[0] == '-')
	{
		std::string empty;
		setPassword(empty);
	}
	else if (arg[0] == '+' && msg_parsed.size() > 1)
		setPassword(msg_parsed[1]);
	else if (msg_parsed.size() == 1)
	{
		std::string mode_err(message_formatter(461, user.getNick(), _name, "Not enough parameters"));
		send(user.getFd(), mode_err.c_str(), mode_err.size(), 0);
		return ;
	}
	msg_parsed.erase(msg_parsed.begin() + 1);
	std::string mode(arg + ' ' + this->getPassword());
	std::string reply = mode_msg_formatter(user, mode, this->_name);
	this->writeToChannel(reply, "");
}

int	Channel::modeMaxUsers(std::vector<std::string>& msg_parsed, std::string& arg, User& user)
{
	if (arg[0] == '+' && msg_parsed.size() == 1)
	{
		std::string mode_err = "461 " + user.getNick() + " MODE: need more params\r\n";
		send(user.getFd(), mode_err.c_str(), mode_err.size(), MSG_NOSIGNAL);
		return 1;
	}
	else if (arg[0] == '+' && msg_parsed.size() > 1)
	{
		if (!is_numeric(msg_parsed[1].c_str()))
		{
			int num = atoi(msg_parsed[1].c_str());
			setMaxUsers(num);
			msg_parsed.erase(msg_parsed.begin() + 1);
		}
		else
		{
			std::string mode_err = "461 " + user.getNick() + " MODE: need more params\r\n";
			send(user.getFd(), mode_err.c_str(), mode_err.size(), MSG_NOSIGNAL);
			return 1;
		}
	}
	else if (arg[0] == '-')
		setMaxUsers(0);
	std::stringstream oss;
	oss << this->getMaxUsers();
	std::string mode(arg + ' ' + oss.str());
	std::string reply = mode_msg_formatter(user, mode, this->_name);
	this->writeToChannel(reply, "");
	return 0;
}

int	Channel::modeOperator(std::string& arg, User& user, User* new_operator)
{
	if(std::find(this->_user_vector.begin(), this->_user_vector.end(), *new_operator) == this->_user_vector.end())
	{
		std::string mode_err = ":server 441" + user.getNick()
		+ ' ' + new_operator->getNick()
		+ ' ' + _name + " :They aren't on that channel\r\n";
		send(user.getFd(), mode_err.c_str(), mode_err.size(), MSG_NOSIGNAL);
		return 1;
	}
	if (arg[0] == '-')
	{
		if (isInVector(*new_operator, this->_operators_vector))
			this->_operators_vector.erase(std::find(_operators_vector.begin(), _operators_vector.end(), *new_operator));
	}
	else if (arg[0] == '+')
	{
		if (!isInVector(*new_operator, this->_operators_vector))
			this->_operators_vector.push_back(*new_operator);
	}
	std::string confirm = ":server MODE #" + _name + ' ' + arg + ' ' + new_operator->getNick() + "\r\n";
	writeToChannel(confirm, "");
	return 0;
}

void	Channel::modeTopic(std::string& arg, User& user)
{
	if (arg[0] == '-')
		setTopicRestriction(false);
	else if (arg[0] == '+')
		setTopicRestriction(true);
	std::string reply = mode_msg_formatter(user, arg, this->_name);
	this->writeToChannel(reply, "");
}