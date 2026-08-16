#ifndef IRCBOT_HPP
#define IRCBOT_HPP

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctime>
#include "Parser.hpp"
#include "Dispatcher.hpp"
#include "exception"
#include "stdexcept"

class IrcBot {
private:
    std::string server_ip;
    std::string password;
    int server_port;
    int socket_fd;

public:
    IrcBot(const std::string& ip, int port, const std::string& pass);
    ~IrcBot();

    void    connectToServer();
    void    authenticate();
    void    run();
    void    handleMessage(std::string& message);
};

#endif