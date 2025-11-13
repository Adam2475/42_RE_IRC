#ifndef SERVER_HPP
#define SERVER_HPP

#include "header.hpp"
#include "User.hpp"
#include "Channel.hpp"

class Server
{
    private:
        unsigned short          _port;
        std::string             _password;
        int                     _serv_fd;
        std::vector<pollfd>     _poll_fds;
        std::vector<User>       _users;
        std::vector<Channel>    _channels;
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
        void        server_start();
        void        start_main_loop();
        void        handle_new_connection(struct pollfd *tmp, int client_socket);
        User	    *getUserByFd(int clientSocket);
        User        findUserByNick(std::string targetNick);
        void        disconnectClient(int clientSocket);
        int         authenticate_user(std::vector<std::string> parsed_message, User *sending_user);
        void        check_authentication(User *sending_user);
        int         check_already_registered(std::vector<std::string> parsed_message, User *sending_user);
        int         check_commands(std::vector<std::string> parsed_message, User *sending_user);
		int		    channelAdder(std::string& channelName, User& user, std::string& pass);
        void        channelCreate(std::string& channelName, std::string& pass, User& user);
        Channel*    findChannelByName(std::string channelName);

        ///////////////////
        // Commands
        ///////////////////
        int cmdJoin(std::vector<std::string>& mess, User &user);
        int cmdQuit(std::vector<std::string> parsed_message, User &user);
        int cmdPrivateMsg(std::vector<std::string> parsed_message, User &user);
        int cmdPart(std::vector<std::string> parsed_message, User &user);
        int cmdInvite(std::vector<std::string> parsed_message, User &user);
        int cmdTopic(std::vector<std::string> parsed_message, User &user);
};

std::string message_formatter2(int error, std::string command, const char* message);

#endif