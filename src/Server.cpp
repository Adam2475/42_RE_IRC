#include "../inc/Server.hpp"

////////////////////////
// Constructors
////////////////////////

Server::Server() {}

Server::Server(short int port, std::string password) : _port(port), _password(password)
{}

Server::~Server() {}

////////////////////////
// Public Methods
////////////////////////

void Server::disconnectClient(int clientSocket)
{
	// User quittingUser = findUserByFd(clientSocket);
    // if (quittingUser.getNick().empty())
	// {
    //     // User may not be fully registered, or already cleaned up.
    //     // Just ensure the socket is closed and removed from poll list.
    //     for (size_t i = 0; i < _poll_fds.size(); ++i)
	// 	{
    //         if (_poll_fds[i].fd == clientSocket)
	// 		{
    //             close(_poll_fds[i].fd);
    //             _poll_fds.erase(_poll_fds.begin() + i);
    //             break;
    //         }
    //     }
    //     return;
    // }

	// Erase the user from all channels and notify members
    // for (std::vector<Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	// {
    //     if (isInVector(quittingUser, it->getUserVector()))
	// 	{
    //         //it->writeToChannel(quittingUser, out);
    //         it->partUser(quittingUser, *it, out);
    //     }
    // }

	for (size_t i = 0; i < _poll_fds.size(); ++i)
	{
        if (_poll_fds[i].fd == clientSocket)
		{
            close(_poll_fds[i].fd);
            _poll_fds.erase(_poll_fds.begin() + i);
            break;
        }
    }

    // Remove user from _users vector
    for (size_t i = 0; i < _users.size(); ++i)
	{
        if (_users[i].getFd() == clientSocket)
		{
            _users.erase(_users.begin() + i);
            break;
        }
    }

	std::cout << "Client " << clientSocket << ") disconnected." << std::endl;

}

// User Server::findUserByFd(int clientSocket)
// {
// 	User targetUser;
// 	for (size_t ui = 0; ui < _users.size(); ++ui)
// 	{
// 		if (_users[ui].getFd() == clientSocket)
// 		{
// 			targetUser = _users[ui]; // copy
// 		}
// 	}
// 	return targetUser;
// }

User *Server::getUserByFd(int clientSocket)
{
	for (size_t i = 0; i < _users.size(); ++i)
	{
		if (_users[i].getFd() == clientSocket)
			return (&_users[i]);
	}
	return NULL;
}

void Server::handle_new_connection(struct pollfd *tmp, int client_socket)
{
	init_pollfd(tmp, client_socket);
	// /tmp->events |= POLLRDHUP;
	_poll_fds.push_back(*tmp);
	User new_user("", "", client_socket);
	_users.push_back(new_user);
}

static int exit_code = 0;

void signalHandler(int sig)
{
	exit_code = sig;
}

void Server::shutdown_server()
{
	std::cout << "shutdown requested, cleaning up..." << std::endl;
    // Close all fds registered in poll and clear containers so destructors free memory.
    for (size_t i = 0; i < _poll_fds.size(); ++i)
    {
        if (_poll_fds[i].fd > 0)
            close(_poll_fds[i].fd);
    }
    _poll_fds.clear();
    _users.clear();
    // Also ensure listening socket closed
    if (_serv_fd > 0)
    {
        close(_serv_fd);
        _serv_fd = -1;
    }
    exit(exit_code);
}

void Server::start_main_loop()
{
	struct pollfd					tmp;
	int								client_socket;
	int								status;
	char							buffer[1024];
	//std::vector<pollfd>::iterator	it;
	//tmp.events |= POLLRDHUP;

	init_pollfd(&tmp, _serv_fd);
	_poll_fds.push_back(tmp);

	while (true)
	{
		// implement signals
		// SIGINT ctrl + C
		
		signal(SIGINT, signalHandler);

		if (exit_code)
			shutdown_server();

		if (poll(_poll_fds.data(), _poll_fds.size(), 0) == -1)
			return ;

		if (_poll_fds[0].revents & POLLIN)
		{
			client_socket = accept(_serv_fd, NULL, NULL);
			std::cout << client_socket << std::endl;
			if (client_socket > 0)
				handle_new_connection(&tmp, client_socket);
		}

		for (int i = 1; i < (int)_poll_fds.size(); i++)
		{
			if (_poll_fds[i].revents & POLLIN)
			{
				bzero(buffer, sizeof(buffer));
				status = recv(_poll_fds[i].fd, buffer, sizeof(buffer) - 1, 0);
				std::string tmp(buffer);
				std::vector<std::string> parsed_message;
				
				if (status > 0)
				{
					User *sending_user;
					sending_user = getUserByFd(_poll_fds[i].fd);
					buffer[status] = '\0';
					parsed_message = parse_message(buffer);

					if (clearStrCRFL(tmp) == 1)
						continue ;
					
					if (!sending_user->isActive())
					{
						if (parsed_message.empty())
							continue ;
						if (parsed_message[0] == "PASS")
						{
							sending_user->setPswdFlag(true);
						}
						else if (parsed_message[0] == "NICK")
						{
							sending_user->setNick(parsed_message[1]);
						}
						else if (parsed_message[0] == "USER")
						{
							sending_user->setUser(parsed_message[1]);
						}

						if (sending_user->getPswdFlag() == true && 
							!sending_user->getNick().empty() &&
							!sending_user->getUser().empty())
						{
							std::cout << "user authenticated correctly" << std::endl;
							sending_user->setActive(true);
						}
						else
						{
							std::cout << "user is not active" << std::endl;
						}
					}
					else
					{
						// reimplement commands
						std::cout << buffer << std::endl;
						std::cout << "user is active" << std::endl;
					}
				}
				// handle client disconnection
				else if (status == 0)
				{
					disconnectClient(_poll_fds[i].fd);
					std::cout << "client disconnected" << std::endl;
				}
			}
		}
	}
}

void Server::server_start()
{
    sockaddr_in serv_addr;

    init_address(&serv_addr, _port);
    if (create_socket(&_serv_fd))
		throw std::runtime_error("failed to create socket");
	if (bind_socket(&_serv_fd, &serv_addr))
		throw std::runtime_error("failed binding the socket");
	if (set_socket_listen(&_serv_fd))
		throw std::runtime_error("failed setting socket on listening");

	std::cout << "Server listening on port: " << ntohs(serv_addr.sin_port) << std::endl;

	try
	{
		start_main_loop();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
}