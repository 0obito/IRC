#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
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
    int                                     nicknameOwner(std::string& nickname);

    // modifiers
    int         acceptNewClient();
    void        addChannel(Channel* channel);
    void        removeChannel(const std::string& name);

    // other
    void        handeleDisconnect(int fd);
    void        handeleDisconnect(std::map<int, Client>::iterator);
    ssize_t     send_message(int fd, std::string& buf);
    void        pingPong();
    void        disconnect();
};

extern int  signal_status;

#endif
