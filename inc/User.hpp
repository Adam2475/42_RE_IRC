#ifndef USER_HPP
#define USER_HPP
#include <iostream>
#include <vector>

class User
{
    private:
        std::string     _nick;
        std::string     _user;
        int             _fd;
        bool            _is_active;
        bool            _set_pass;

    public:
        std::string     _buffer;
        User();
        User(std::string nick, std::string user, int fd);
        ~User();
        
        //////////////////////
        // Public Methods
        //////////////////////
        bool        isActive();
		bool operator==(const User& other) const;

        //////////////////////
        // Getters & Setters
        //////////////////////
        bool        getPswdFlag() const;
        void        setPswdFlag(bool value);
        int         getFd() const;
        void        setNick(std::string nick);
        std::string getNick() const;
        void        setUser(std::string user);
        std::string getUser() const;
        void        setActive(bool value);
};

int     check_existing_user(std::vector<User> users, std::string username);
bool    isInVector(User& user, const std::vector<User>& vector);

#endif