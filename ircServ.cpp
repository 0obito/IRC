#include <iostream>
#include <string>
#include <sstream>
#include "ircServ.hpp"

int signal_status = 0;

void    signal_handler(int signal) {
    signal_status = signal;
}

int main(int ac, char *av[]) {
    int port;

    if (ac != 3) {
        std::cerr << "[ERROR] Incorrect format. Try:" << std::endl << "./ircserv <port> <password>\n";
        return 1;
    }
    std::stringstream ss(av[1]);
    ss >> port;
    if (ss.fail() || !ss.eof()) {
        std::cerr << "[ERROR] Port number is invalid!\n";
        return 1;
    }
    if (port < 1024 || port > 65535) {
        std::cerr << "[ERROR] Port number should be between 1024 and 65535!\n";
        return 1;
    }
    std::cout << "[SUCCESS] Starting server on port " << port << "...\n";
    std::signal(SIGINT, signal_handler);
    std::signal(SIGQUIT, signal_handler);
    Server server(port, av[2]);

    return 0;
}
