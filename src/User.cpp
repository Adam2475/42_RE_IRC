#include "../inc/User.hpp"

User::User() {}

User::User(std::string nick, std::string user, int fd) : _nick(nick), _user(user), _fd(fd)
{
    _is_active = false;
    _set_pass = false;
}

User::~User() {}

///////////////////////
// Getters & Setters
///////////////////////

void User::setNick(std::string nick)
{
    _nick = nick;
}

void User::setUser(std::string user)
{
    _user = user;
}

bool User::getPswdFlag() const
{
    return _set_pass;
}

void User::setPswdFlag(bool value)
{
    _set_pass = value;
}

void User::setActive(bool value)
{
    _is_active = value;
}

bool User::isActive()
{
    return (_is_active == true ? true : false);
}

int User::getFd() const
{
    return _fd;
}

std::string User::getNick() const
{
    return _nick;
}

std::string User::getUser() const
{
    return _user;
}

bool User::operator==(const User& other) const
{
	if (_fd == other.getFd())
		return 1;
	return 0;
}


int check_existing_user(std::vector<User> users, std::string username)
{
	std::vector<User>::iterator it;
	for (it = users.begin(); it != users.end(); it++)
	{
		if (it->getNick() == username)
		{
			std::cout << "username: " << username << " checked nick: " << it->getNick() << std::endl;
			return (1);
		}
	}
	return (0);
}

bool	isInVector(User& user, const std::vector<User>& vector)
{
	for (size_t i = 0; i < vector.size(); i++)
	{
		if (vector[i] == user)
			return 1;
	}
	return 0;	
}
