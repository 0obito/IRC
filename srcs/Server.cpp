#include "includes/Server.hpp"

std::string Server::get_password(){
    return(password);
}

void Server::initserver()
{
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;
}

void    Server::run()
{
    serversocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    int opt = 1;
    setsockopt(serversocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    bind(serversocket, (sockaddr*)&address, sizeof(address));
    listen(serversocket, SOMAXCONN);
}

void    Server::multiplexar()
{
    epfd = epoll_create1(0);
    if (epfd < 0)
    {
        std::cerr << "ERROR: epoll_create1()" << std::endl;
        exit(1);
    }
    ev.events = EPOLLIN;
    ev.data.fd = serversocket;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serversocket, &ev);
}

Server::Server(int port_in, std::string pwd) {
    port = port_in;
    password = pwd;
    initserver();
    run();
}

Server::Server(const Server& other) {
    *this = other;
}

const Server& Server::operator=(const Server& other) {
    if (this == &other)
        return(*this);
    port = other.port;
    password = other.password;
    return(*this);
}

Server::~Server() {
    close(serversocket);
}