#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>

class Server {
private:
    int                     serversocket;
    int                     port;
    struct sockaddr_in      address;
    std::string             password;
    // std::vector<pollfd>     fds;

public:
    Server(int port_in, std::string pwd);
    Server(const Server& other);
    const Server& operator=(const Server& other);
    ~Server();

    void    initserver();
    void    run();
    void    acceptNewClient();
};

#endif