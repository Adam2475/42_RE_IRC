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
        void    handle_new_connection(struct pollfd *tmp, int client_socket);
        User	*getUserByFd(int clientSocket);
        //User    findUserByFd(int clientSocket);
        void    disconnectClient(int clientSocket);
        // void    shutdown_server();
        int     authenticate_user(std::vector<std::string> parsed_message, User *sending_user);
        void    check_authentication(User *sending_user);
};

#endif