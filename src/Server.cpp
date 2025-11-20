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

void Server::remove_from_pollfds(int clientSocket)
{
	for (size_t i = 0; i < _poll_fds.size(); ++i)
	{
        if (_poll_fds[i].fd == clientSocket)
		{
            close(_poll_fds[i].fd);
            _poll_fds.erase(_poll_fds.begin() + i);
            break;
        }
    }
}

void Server::remove_from_user_vector(int clientSocket)
{
    for (size_t i = 0; i < _users.size(); ++i)
	{
        if (_users[i].getFd() == clientSocket)
		{
            _users.erase(_users.begin() + i);
            break;
        }
    }	
}

void Server::remove_user_from_channels(int clientSocket, std::string quit_msg)
{
	User *quittingUser = getUserByFd(clientSocket);

    for (std::vector<Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
        if (isInVector(*quittingUser, it->getUserVector()))
            it->partUser(*quittingUser, *it, quit_msg, QUIT);
    }	
}

void Server::disconnectClient(int clientSocket, std::string quit_msg)
{
	remove_user_from_channels(clientSocket, quit_msg);
	remove_from_pollfds(clientSocket);
	remove_from_user_vector(clientSocket);
	std::cout << "Client no: " << clientSocket << " disconnected" << std::endl;
}

User *Server::getUserByFd(int clientSocket)
{
	for (size_t i = 0; i < _users.size(); ++i)
	{
		if (_users[i].getFd() == clientSocket)
			return (&_users[i]);
	}
	return NULL;
}

void Server::check_authentication(User *sending_user)
{
	if (sending_user->getPswdFlag() == true && 
		!sending_user->getNick().empty() &&
		!sending_user->getUser().empty())
	{
		std::string message;
		message += ":server 001 ";
		message += sending_user->getNick();
		message += " :Welcome to 42_IRC, ";
		message += sending_user->getNick();
		message += "\r\n";
		send(sending_user->getFd(), message.c_str(), message.size(), 0);
		sending_user->setActive(true);
	}
}

Channel*	Server::findChannelByName(std::string channelName)
{
 	for (std::vector<Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
        if (it->getName() == channelName)
		{
            return &(*it);
        }
    }
	return NULL;
}

void Server::handle_new_connection(struct pollfd *tmp, int client_socket)
{
	init_pollfd(tmp, client_socket);
	_poll_fds.push_back(*tmp);
	User new_user("", "", client_socket);
	_users.push_back(new_user);
}

static int exit_code = 0;

void signalHandler(int sig)
{
	exit_code = sig;
}

std::vector<std::string> stringSplit(const std::string& s, char delim)
{
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;

    while (std::getline(ss, item, delim))
        result.push_back(item);

    return result;
}


int Server::multipleJoin(std::vector<std::string>& channelsNames, User& sendingUser)
{
	std::vector<std::string> joinVect;
	for (size_t i = 0; i < channelsNames.size(); i++)
	{
		joinVect.push_back("JOIN");
		joinVect.push_back(channelsNames[i]);
		cmdJoin(joinVect, sendingUser);
		joinVect.clear();
	}
	return 0;
}

int Server::multipleJoinPass(std::vector<std::string>& channelsNames, std::vector<std::string>& passwds, User& sendingUser)
{
	std::map<std::string, std::string> channelKeyMap;
	if (passwds.size() == 1)
	{
		size_t i = 0;
		while (i < channelsNames.size())
			channelKeyMap.insert(std::make_pair(channelsNames[i++], passwds[0]));
	}
	else
	{
		size_t i = 0;
		while (i < channelsNames.size() && i < passwds.size())
		{
			channelKeyMap.insert(std::make_pair(channelsNames[i], passwds[i]));
			i++;
		}
		while (i < channelsNames.size())
		{
			channelKeyMap.insert(std::make_pair(channelsNames[i], ""));
			i++;
		}
	}
	std::vector<std::string> joinVect;
	for (std::map<std::string, std::string>::iterator it = channelKeyMap.begin(); it != channelKeyMap.end(); ++it)
	{
		joinVect.push_back("JOIN");
		joinVect.push_back(it->first);
		joinVect.push_back(it->second);
		cmdJoin(joinVect, sendingUser);
		joinVect.clear();
	}
	return 0;
}


int Server::check_commands(std::vector<std::string> parsed_message, User *sending_user)
{
	if (parsed_message[0] == "JOIN")
	{
		std::vector<std::string> multiple_channels;
		std::vector<std::string> multiple_pass;
		if (parsed_message.size() > 1)
			multiple_channels = stringSplit(parsed_message[1], ',');
		if (parsed_message.size() > 2)
			multiple_pass = stringSplit(parsed_message[2], ',');
		multipleJoinPass(multiple_channels, multiple_pass, *sending_user);
		return (1);
	}
	if (parsed_message[0] == "QUIT")
	{
		cmdQuit(parsed_message, *sending_user);
		return (1);
	}
	if (parsed_message[0] == "PRIVMSG")
	{
		cmdPrivateMsg(parsed_message, *sending_user);
		return (1);
	}
	if (parsed_message[0] == "PART")
	{
		cmdPart(parsed_message, *sending_user);
		return (1);
	}
	if (parsed_message[0] == "INVITE")
	{
		cmdInvite(parsed_message, *sending_user);
		return (1);
	}
	if (parsed_message[0] == "MODE")
	{
		cmdMode(parsed_message, *sending_user);
		return (1);
	}
	if (parsed_message[0] == "PING")
	{
		cmdPing(parsed_message, *sending_user);
		return (1);
	}
	if (parsed_message[0] == "TOPIC")
	{
		cmdTopic(parsed_message, *sending_user);
		return (1);
	}
	if (parsed_message[0] == "KICK")
	{
		cmdKick(parsed_message, *sending_user);
		return (1);
	}
	if (parsed_message[0] == "WHO")
	{
		cmdWho(parsed_message, *sending_user);
		return (1);
	}
	return (0);
}

int Server::authenticate_user(std::vector<std::string> parsed_message, User *sending_user)
{
	if (parsed_message.empty())
		return (1);
	if (parsed_message[0] == "PASS")
	{
		if (parsed_message.size() < 2 || parsed_message[1].empty())
		{
			std::string message;
			message += ":server 464 ";
			message += sending_user->getNick().empty() ? "*" : sending_user->getNick();
			message += " :Password Incorrect\n\r";
			send(sending_user->getFd(), message.c_str(), message.size(), 0);
			return (1);
		}
		if (parsed_message[1] == _password)
		{
			sending_user->setPswdFlag(true);
		}
		else
		{
			std::string message;
			message += ":server 464 ";
			message += sending_user->getNick().empty() ? "*" : sending_user->getNick();
			message += " :Password Incorrect\n\r";
			send(sending_user->getFd(), message.c_str(), message.size(), 0);
			return (1);
		}
	}
	else if (parsed_message[0] == "NICK")
	{
		if (cmdNick(parsed_message, sending_user))
			return (1);
	}
	else if (parsed_message[0] == "USER")
	{
		if (parsed_message.size() != 5)
		{
			std::string message;
			message += ":server 461 ";
			message += sending_user->getNick().empty() ? "*" : sending_user->getNick();
			message += " :USER :Not enough parameters\n\r";
			send(sending_user->getFd(), message.c_str(), message.size(), 0);
			return (1);
		}
		else
		{
			sending_user->setUser(parsed_message[1]);
		}
	}
	check_authentication(sending_user);
	return (0);
}

int Server::handle_commands(std::vector<std::string> parsed_message, User *sending_user)
{
	std::string out;

	if (!sending_user->isActive())
	{
		if (authenticate_user(parsed_message, sending_user))
			return (1);
	}
	else
	{
		if (check_already_registered(parsed_message, sending_user) || check_commands(parsed_message, sending_user))
		{
			return (1);
		}
		else
		{
			out += ":server 421 :";
			out += sending_user->getNick() + " ";
			out += parsed_message[0];
			out += " :Unknown command";
			out += "\r\n";
			std::cout << parsed_message[0] << " :Command not found" << std::endl;
			send(sending_user->getFd(), out.c_str(), out.size(), 0);
		}
	}
	return (0);
}

int Server::check_already_registered(std::vector<std::string> parsed_message, User *sending_user)
{
	if (parsed_message[0] == "PASS" || parsed_message[0] == "USER")
	{
		std::string message;
		message += ":server 462 ";
		message += sending_user->getNick();
		message += " :You may not reregister\n\r";
		send(sending_user->getFd(), message.c_str(), message.size(), 0);
		return (1);
	}
	if (parsed_message[0] == "NICK")
		return (cmdNick(parsed_message, sending_user), 1);
	return (0);
}

void Server::start_main_loop()
{
	struct pollfd					tmp;
	int								client_socket;
	int								status;
	char							buffer[1024];

	init_pollfd(&tmp, _serv_fd);
	_poll_fds.push_back(tmp);

	while (true)
	{
		signal(SIGINT, signalHandler);
		//signal(SIGQUIT, signalHandler);
		signal(SIGTSTP, signalHandler);

		if (exit_code)
		{
			close(_serv_fd);
			break ;
		}

		if (poll(_poll_fds.data(), _poll_fds.size(), 0) == -1)
			return ;

		if (_poll_fds[0].revents & POLLIN)
		{
			client_socket = accept(_serv_fd, NULL, NULL);
			// std::cout << client_socket << std::endl;
			if (client_socket > 0)
				handle_new_connection(&tmp, client_socket);
		}

		for (int i = 1; i < (int)_poll_fds.size(); i++)
		{
			if (_poll_fds[i].revents & POLLIN)
			{
				User *sending_user = getUserByFd(_poll_fds[i].fd);
				bzero(buffer, sizeof(buffer));
				status = recv(_poll_fds[i].fd, buffer, sizeof(buffer) - 1, 0);
				
				if (status > 0)
				{
					// append message to user buffer
					// append message to user buffer
					int user_fd = _poll_fds[i].fd;
					sending_user->_buffer.append(buffer, (size_t)status);
					
					// search newline in the string
					std::string tmp = sending_user->_buffer;
					size_t newline_pos;
					bool user_still_exists = true;

					while ((newline_pos = tmp.find('\n')) != std::string::npos)
					{

                        // take up to the newline (inclusive) and remove from buffer
                        std::string line = tmp.substr(0, newline_pos + 1);
                        tmp.erase(0, newline_pos + 1);

						// strip trailing CR/LF
                        while (!line.empty() && (line[line.size() - 1] == '\r' || line[line.size() - 1] == '\n'))
                            line.erase(line.end() - 1);

                        if (line.empty())
                            continue;

						std::vector<std::string> parsed_message = parse_message(line);

						//std::cout << line << std::endl;
						
						if (handle_commands(parsed_message, sending_user))
						{
							// command was handled; the command may have disconnected the user
							if (getUserByFd(user_fd) == NULL)
							{
								user_still_exists = false;
								break;
							}
							// user still present, continue processing next line
							continue;
						}
					}
				// save any remaining partial data back into the user's buffer
				if (user_still_exists)
				{
					User *maybe_user = getUserByFd(user_fd);
					if (maybe_user)
						maybe_user->_buffer = tmp;
				}
				}
				// handle client disconnection
				else if (status == 0)
				{
					// process partial commands before disconnecting
					disconnectClient(_poll_fds[i].fd, ":Client Quit");
					//std::cout << "client disconnected" << std::endl;
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
	try {
		start_main_loop();
	}
	catch(const std::exception& e) {
		std::cerr << e.what() << '\n';
	}
}