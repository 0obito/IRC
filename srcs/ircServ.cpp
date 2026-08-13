#include "../includes/ircServ.hpp"

int signalStatus = 0;

void    signalHandler(int signal) {
    signalStatus = signal;
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
    std::cout << "Starting server on port " << port << "...\n";
    std::signal(SIGINT, signalHandler);
    std::signal(SIGQUIT, signalHandler);
    try {
        Server server(port, av[2]);
        server.initServer();
        server.multiplexer();
    }
    catch (std::exception &e) {
        std::cerr<< "[ERROR] " << e.what()<<std::endl;
    }
    return 0;
}
