#include "../inc/Server.hpp"

unsigned short parse_port(char *port_str)
{
    int port;

    if (is_numeric(port_str) || std::strlen(port_str) >= 9)
        invalid_port_error();
    port = atoi(port_str);
    if (port <= 0 || port >= 65535)
        invalid_port_error();
    return (port);
}

int main(int ac, char **av)
{
    unsigned short  port;
    std::string     password;

    if (ac != 3)
        return (std::cerr << "usage: ./ircserv <port> <password>" << std::endl, 1);

    port = parse_port(av[1]);
    password = av[2];

    try
    {
        Server server(port, password);
        server.server_start();
    }
    catch(const std::exception &e)
    {
        std::cerr << "error initializing the server: " << e.what() << std::endl;
    }
    return (0);
}