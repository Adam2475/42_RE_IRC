#ifndef USER_HPP
#define USER_HPP

#include "header.hpp"

class User
{
    private:
        std::string     _nick;
        std::string     _user;
        //struct pollfd   _pollfd;
        int             _fd;
        bool            _is_active;
        bool            _set_pass;
        // bool            _set_nick;
        // bool            _set_user;
    public:
        User();
        User(std::string nick, std::string user, int fd);
        ~User();

        //////////////////////
        // Public Methods
        //////////////////////
        bool        isActive();

        //////////////////////
        // Getters & Setters
        //////////////////////
        bool        getPswdFlag();
        void        setPswdFlag(bool value);
        int         getFd();
        void        setNick(std::string nick);
        std::string getNick();
        void        setUser(std::string user);
        std::string getUser();
        void        setActive(bool value);
};

#endif