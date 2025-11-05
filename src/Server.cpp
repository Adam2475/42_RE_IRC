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

User *Server::getUserByFd(int clientSocket)
{
	for (size_t i = 0; i < _users.size(); ++i)
	{
		if (_users[i].getFd() == clientSocket)
		{
			// returning reference
			return (&_users[i]);
		}
	}
	return NULL;
}

void Server::start_main_loop()
{
	struct pollfd					tmp;
	int								client_socket;
	int								status;
	char							buffer[1024];
	std::vector<pollfd>::iterator	it;

	// creating and inserting the server pollfd
	init_pollfd(&tmp, _serv_fd);
	_poll_fds.push_back(tmp);

	while (true)
	{
		if (poll(_poll_fds.data(), _poll_fds.size(), -1) == -1)
		{
			return ;
		}

		// checking the server pollfd
		if (_poll_fds[0].revents & POLLIN)
		{
			std::cout << "incoming event received" << std::endl;
			client_socket = accept(_serv_fd, NULL, NULL);

			if (client_socket > 0)
			{
				std::cout << "new connection accepted" << std::endl;
				pollfd tmp;
				init_pollfd(&tmp, client_socket);
				_poll_fds.push_back(tmp);
				// create new user
				User new_user("", "", client_socket);
				_users.push_back(new_user);
			}
		}

		for (it = _poll_fds.begin() + 1; it != _poll_fds.end(); *it++)
		{
			if (it->revents & POLLIN)
			{
				std::cout << "message received from client" << std::endl;
				//std::cout << it->fd << std::endl;
				bzero(buffer, sizeof(buffer));
				// returns the number of byte read or 0 if disconnect
				status = recv(it->fd, buffer, sizeof(buffer) - 1, 0);
				//std::cout << buffer << std::endl;
				//std::cout << status << std::endl;
				if (status > 0)
				{
					User *sending_user;
					sending_user = getUserByFd(it->fd);
	
					std::cout << sending_user->getFd() << std::endl;

					// message building
					buffer[status] = '\0';
					std::string message(buffer);
					std::stringstream oss(message);

					std::string command;
					oss >> command;
	
					if (!sending_user->isActive())
					{
						if (command == "PASS")
						{
							std::cout << command << std::endl;
							sending_user->setPswdFlag(true);
						}
						else if (command == "NICK")
						{
							std::string tmp;
							oss >> tmp;
							std::cout << command << std::endl;
							std::cout << tmp << std::endl;
							sending_user->setNick(tmp);
						}
						else if (command == "USER")
						{
							std::string tmp;
							oss >> tmp;
							std::cout << command << std::endl;
							std::cout << tmp << std::endl;
							sending_user->setUser(tmp);
						}

						if (sending_user->getPswdFlag() == true && 
							!sending_user->getNick().empty() &&
							!sending_user->getUser().empty())
						{
							sending_user->setActive(true);
						}

						std::cout << "user is not active" << std::endl;
					}
					else
					{
						std::cout << buffer << std::endl;
						std::cout << "user is active" << std::endl;
					}
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