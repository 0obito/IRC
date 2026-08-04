#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include "./Channel.hpp"
#include "./Client.hpp"

class Server {
private:
    static const int                MAX_EVENTS = 100;
    std::string                     serverName;
    int                             serversocket;
    int                             port;
    std::string                     password;
    struct sockaddr_in              address;
    int                             epfd;
    struct epoll_event              event_buffer[MAX_EVENTS];
    std::map<int, Client>           clientMap;
    std::map<std::string, Channel*> _channels;

public:
    // canonical form
    Server(int port_in, const std::string& pwd);
    ~Server();

    // server backbone
    void        initserver();
    void        run();
    void        multiplexar();

    // getters
    const std::string&                      getServerName() const;
    const std::string&                      getPassword() const;
    int                                     get_epfd() const;
    std::map<int, Client>&                  getMap();
    Channel*                                getChannel(const std::string& name);
    std::map<std::string, Channel*>&        getChannels();

    // bool getters
    int         isNicknameTaken(std::string& nickname);

    // modifiers
    int         acceptNewClient();
    void        addChannel(Channel* channel);
    void        removeChannel(const std::string& name);

    // other
    void        handeleDisconnect(int fd);
    ssize_t     send_message(int fd, std::string& buf);
    void        pingPong();
};

#endif /*SERVER_HPP*/
