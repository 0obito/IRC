#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <map>

class Server {
private:
    int                     serversocket;
    int                     epfd;
    int                     port;
    struct sockaddr_in      address;
    struct epoll_event      ev;
    std::string             password;
    std::map<int, std::string> client;
    // std::vector<pollfd>     fds;

public:
    Server(int port_in, std::string pwd);
    Server(const Server& other);
    const Server& operator=(const Server& other);
    ~Server();

    std::string get_password();
    void        add_client(int fd);
    bool        isNicknameTaken(const std::string& nickname) const;
    void        initserver();
    void        run();
    void        multiplexar();
    void        acceptNewClient();
};

#endif