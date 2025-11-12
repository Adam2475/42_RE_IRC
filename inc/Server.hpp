#ifndef SERVER_HPP
#define SERVER_HPP

#include "header.hpp"
#include "User.hpp"
#include "Channel.hpp"

class Server
{
    private:
        unsigned short      _port;
        std::string         _password;
        int                 _serv_fd;
        // vector of fds that will be checked by poll
        std::vector<pollfd> _poll_fds;
        std::vector<User>   _users;
        std::vector<Channel>   _channels;
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

		int		channelAdder(std::string& channelName, User& user, std::string& pass);
        void    channelCreate(std::string& channelName, std::string& pass, User& user);


        ///////////////////
        // Commands
        ///////////////////
        int cmdJoin(std::vector<std::string>& mess, User &user);
};

#endif