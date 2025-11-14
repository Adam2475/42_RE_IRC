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
#include <algorithm>
#include <sstream>
// #include <fcntl.h>
#include <csignal>
// header should only be included elswhere for prototypes
#include "User.hpp"

///////////////
// Colors
//////////////

// --------- COLORI TESTO ---------
#define BLACK   "\033[30m"
#define RED   "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW   "\033[33m"
#define BLUE   "\033[34m"
#define MAGENTA   "\033[35m"
#define CYAN   "\033[36m"
#define WHITE   "\033[37m"

// --------- STILI TESTO ---------
#define BOLD  "\033[1m"
#define UND   "\033[4m"
#define BLINK "\033[5m"

// --------- COLORI SFONDO ---------
#define BBLK  "\033[40m"
#define BRED  "\033[41m"
#define BGRN  "\033[42m"
#define BYEL  "\033[43m"
#define BBLU  "\033[44m"
#define BMAG  "\033[45m"
#define BCYN  "\033[46m"
#define BWHT  "\033[47m"

// --------- RESET ---------
#define RESET "\033[0m"

// for telling part user which reply send to channels
enum e_part
{
    QUIT,
    PART,
};

//////////////////
// Prototypes
//////////////////

bool                        is_numeric(const char *str);
void                        invalid_port_error();
void                        init_address(sockaddr_in *serv_addr, int port);
int                         bind_socket(int *serv_fd, const sockaddr_in *serv_addr);
int                         create_socket(int *serv_fd);
int                         set_socket_listen(int *serv_fd);
void                        init_pollfd(pollfd *tmp, int fd);
//std::vector<std::string>    parse_message(char *buffer);
std::vector<std::string>    parse_message(std::string buffer);
int                         clearStrCRFL(std::string& received);
void                        signalHandler(int sig);
std::string                 message_formatter(int error, const std::string& nickname, const std::string& channel, const char* message);
bool	                    isStrNotPrintable(const char *str);
int                         removeInitialHash(std::string *target);

#endif