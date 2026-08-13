#ifndef SERVER_HPP
#define SERVER_HPP

#define MAX_EVENTS 100

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <csignal>
#include <cstring>

#include <map>
#include <iostream>
#include <string>


/*
 * Server
 * The project's backbone.
 * It containes everything server specific/related.
 */

class Channel;
class Client;

class Server {
private:
    Server();
    int                             port;
    std::string                     password;
    std::string                     serverName;
    int                             serverSocket;
    struct sockaddr_in              address;
    int                             epfd;
    struct epoll_event              eventBuffer[MAX_EVENTS];
    std::map<int, Client>           clientMap;
    std::map<std::string, Channel*> chansMap;

public:
    // constructor / destructor
    Server(int port_in, const std::string& pwd);
    ~Server();

    // server backbone
    void        initServer();
    void        multiplexer();

    // getters
    const std::string&                      getServerName() const;
    const std::string&                      getPassword() const;
    int                                     getEPFD() const;
    std::map<int, Client>&                  getMap();
    Channel*                                getChannel(const std::string& name);
    std::map<std::string, Channel*>&        getChannels();
    int                                     nicknameOwner(std::string& nickname);

    // modifiers
    int         acceptNewClient();
    void        addChannel(Channel* channel);
    void        removeChannel(const std::string& name);

    // other
    void        disconnectClient(int fd);
    void        disconnectClient(std::map<int, Client>::iterator);
    ssize_t     sendMessage(int fd, std::string& buf);
    void        pingPong();
    void        disconnectServer();
};

extern int  signalStatus;

#endif
