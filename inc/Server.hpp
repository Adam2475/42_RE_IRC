#ifndef SERVER_HPP
#define SERVER_HPP

#include "header.hpp"
#include "User.hpp"

class Server
{
    private:
        unsigned short      _port;
        std::string         _password;
        int                 _serv_fd;
        // vector of fds that will be checked by poll
        std::vector<pollfd> _poll_fds;
        std::vector<User>   _users;
    public:
        ///////////////////
        // Constructors
        ///////////////////
        Server();
        Server(short int port, std::string password);
        ~Server();

        ///////////////////
        // Public Methods
        ///////////////////
        void    server_start();
        void    start_main_loop();
        User	*getUserByFd(int clientSocket);
};

#endif