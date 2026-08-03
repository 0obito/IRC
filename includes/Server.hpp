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
#include "Client.hpp"   // ??? needs checking
#include "Channel.hpp"  // ??? needs checking

class Server {
private:
    // [??? why max events = 100 ?]
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

    Server(const Server& other);

public:
    // canonical form
    Server(int port_in, std::string pwd);
    // Server(const Server& other);
    const Server& operator=(const Server& other);
    ~Server();

    // getters
    const std::string&                      getServerName() const;
    std::string                             get_password();
    int                                     get_epfd() const;
    std::map<int, Client>&                  mapGetter();
    Channel*                                getChannel(const std::string& name);
    const std::map<std::string, Channel*>&  getChannels() const;

    // bool getters
    int         isNicknameTaken(std::string& nickname);

    // modifiers
    int         acceptNewClient();
    void        addChannel(Channel* channel);
    void        removeChannel(const std::string& name);

    // other
    void        initserver();
    void        run();
    void        handeleDisconnect(int fd);
    ssize_t     send_message(int fd, std::string& buf);
    void        pingPong();
    void        multiplexar();
};

#endif /*SERVER_HPP*/
