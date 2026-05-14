#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <map>
#include "Client.hpp"

class Server {
private:
    int                     serversocket;
    int                     epfd;
    int                     port;
    struct sockaddr_in      address;
    static const int        MAX_EVENTS = 100;
    struct epoll_event      event_buffer[MAX_EVENTS];
    std::string             password;
    std::map<int, Client>   client;
    std::string             serverName;

public:
    Server(int port_in, std::string pwd);
    Server(const Server& other);
    const Server& operator=(const Server& other);
    ~Server();

    std::string get_password();
    const std::string& getServerName() const;
    void        add_client(int fd);
    void        handeleDisconnect(int fd);
    bool        isNicknameTaken(const std::string& nickname) const;
    void        initserver();
    void        run();
    void        multiplexar();
    int         acceptNewClient();
    size_t      send_message(int fd, std::string &buf);
};

#endif