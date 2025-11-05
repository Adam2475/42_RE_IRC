#include "../inc/User.hpp"

User::User() {}

User::User(std::string nick, std::string user, int fd) : _nick(nick), _user(user), _fd(fd)
{
    _is_active = false;
    _set_pass = false;
    // _set_nick = false;
    // _set_user = false;
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

bool User::getPswdFlag()
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

int User::getFd()
{
    return _fd;
}

std::string User::getNick()
{
    return _nick;
}

std::string User::getUser()
{
    return _user;
}