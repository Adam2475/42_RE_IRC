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

// void Server::disconnectClient(int clientSocket, std::string quitMessage)
// {

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

void Server::start_main_loop()
{
	struct pollfd					tmp;
	int								client_socket;
	int								status;
	char							buffer[1024];
	std::vector<pollfd>::iterator	it;
	//tmp.events |= POLLRDHUP;

	init_pollfd(&tmp, _serv_fd);
	_poll_fds.push_back(tmp);

	while (true)
	{
		// implement signals

		if (poll(_poll_fds.data(), _poll_fds.size(), 0) == -1)
			return ;

		if (_poll_fds[0].revents & POLLIN)
		{
			client_socket = accept(_serv_fd, NULL, NULL);
			std::cout << client_socket << std::endl;
			if (client_socket > 0)
				handle_new_connection(&tmp, client_socket);
		}

		for (it = _poll_fds.begin() + 1; it != _poll_fds.end(); *it++)
		{
			if (it->revents & POLLIN)
			{
				bzero(buffer, sizeof(buffer));
				status = recv(it->fd, buffer, sizeof(buffer) - 1, 0);
				std::string tmp(buffer);
				std::vector<std::string> parsed_message;
				
				if (status > 0)
				{
					User *sending_user;
					sending_user = getUserByFd(it->fd);
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