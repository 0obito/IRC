#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <map>
#include "Client.hpp"
#include "Channel.hpp"

class Server {
private:
    int                     serversocket;
    int                     epfd;
    int                     port;
    struct sockaddr_in      address;
    static const int        MAX_EVENTS = 100;
    struct epoll_event      event_buffer[MAX_EVENTS];
    std::string             password;
    std::map<int, Client>   clientMap;
    std::string             serverName;
    std::map<std::string, Channel*> _channels;

public:
    Server(int port_in, std::string pwd);
    Server(const Server& other);
    const Server& operator=(const Server& other);
    ~Server();

    std::string get_password();
    const std::string& getServerName() const;
    int         get_epfd() const;
    void        handeleDisconnect(int fd);
    int         isNicknameTaken(std::string& nickname);
    void        initserver();
    void        run();
    void        multiplexar();
    int         acceptNewClient();
    ssize_t     send_message(int fd, std::string &buf);
    std::map<int, Client>    &mapGetter();
    Channel* getChannel(const std::string& name);
    void addChannel(Channel* channel);
    const std::map<std::string, Channel*>& getChannels() const;
    void removeChannel(const std::string& name);
};

#endif