#include "../inc/User.hpp"

User::User() {}

User::User(std::string nick, std::string user, int fd) : _nick(nick), _user(user), _fd(fd)
{
    _is_active = false;
    _set_pass = false;
    //_wrong_pass = false;
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

// void User::setWrongPswd(bool value)
// {
//     _wrong_pass = value;
// }

void User::setActive(bool value)
{
    _is_active = value;
}

// bool User::getWrongPswd()
// {
//     return _wrong_pass;
// }

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
