#include <iostream>
#include <string>
#include <sstream>
#include "ircServ.hpp"

int main(int ac, char *av[]) {
    int port;

    if (ac != 3) {
        std::cerr << "[ERROR] Incorrect format. Try:" << std::endl << "./ircserv <port> <password>" << std::endl;
        return 1;
    }
    std::stringstream ss(av[1]);
    ss >> port;
    if (ss.fail() || !ss.eof()) {
        std::cerr << "[ERROR] Port number is invalid!" << std::endl;
        return 1;
    }
    if (port < 1024 || port > 65535) {
        std::cerr << "[ERROR] Port number should be between 1024 and 65535!" << std::endl;
        return 1;
    }
    std::cout << "[SUCCESS] Starting server on port " << port << "..." << std::endl;
    Server server(port, av[2]);

}
