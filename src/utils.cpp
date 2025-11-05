#include "../inc/header.hpp"

void init_pollfd(pollfd *tmp, int fd)
{
    tmp->fd = fd;
    tmp->events = POLLIN;
    tmp->revents = 0;
}

int set_socket_listen(int *serv_fd)
{
	if (listen(*serv_fd, SOMAXCONN) != 0)
	{
		close(*serv_fd);
		return (1);
	}
    return (0);
}

int bind_socket(int *serv_fd, const sockaddr_in *serv_addr)
{
    if (bind(*serv_fd, (sockaddr *)serv_addr, sizeof(*serv_addr)) < 0)
	{
		close(*serv_fd);
		return (1);
	}
    return (0);
}

int create_socket(int *serv_fd)
{
    *serv_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (*serv_fd < 0)
		return (1);
    return (0);
}

void init_address(sockaddr_in *serv_addr, int port)
{
	serv_addr->sin_family = AF_INET;
	serv_addr->sin_port = htons(port);
	serv_addr->sin_addr.s_addr = htonl(INADDR_ANY);
}

void invalid_port_error()
{
    std::cerr << "invalid port number!" << std::endl;
    std::exit(EXIT_FAILURE);
}

bool is_numeric(const char *str)
{
    for (int i = 0; i < (int)std::strlen(str); i++)
    {
        if (!std::isdigit(str[i]))
            return (true);
    }
    return (false);
}