#ifndef HEADER_HPP
#define HEADER_HPP

#include <iostream>
#include <cstring> // for strlen
#include <cctype> // for isdigit
#include <cstdlib> // for exit
#include <netinet/in.h> // for sockaddr_in struct
#include <unistd.h> // for clone
#include <sys/socket.h>
#include <poll.h>
#include <vector>
#include <sstream>

bool    is_numeric(const char *str);
void    invalid_port_error();
void    init_address(sockaddr_in *serv_addr, int port);
int     bind_socket(int *serv_fd, const sockaddr_in *serv_addr);
int     create_socket(int *serv_fd);
int     set_socket_listen(int *serv_fd);

void    init_pollfd(pollfd *tmp, int fd);

#endif