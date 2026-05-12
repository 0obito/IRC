#include "includes/Server.hpp"
#include "includes/Parser.hpp"
#include "includes/Dispatcher.hpp"

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

int    Server::acceptNewClient()
{
    int new_fd = accept(serversocket, NULL, NULL);
    client[new_fd] = Client(new_fd);
    return (new_fd);
}

void    Server::handeleDisconnect(int fd)
{
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
    client.erase(fd);
}

void    Server::multiplexar()
{
    struct epoll_event      ev;
    int                     nfds;
    int                     new_fd;
    int                     current_fd;
    char                    buffer[1024];
    size_t                  position;

    epfd = epoll_create1(0);
    if (epfd < 0) {
        std::cerr << "ERROR: epoll_create1()" << std::endl;
        exit(1);
    }
    ev.events = EPOLLIN;
    ev.data.fd = serversocket;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serversocket, &ev);
    while(true) {
        nfds = epoll_wait(epfd, event_buffer, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; i++) {
            current_fd = event_buffer[i].data.fd;
            if (current_fd == serversocket) {
                new_fd = Server::acceptNewClient();
                if (new_fd != 1){
                    struct epoll_event client_ev; 
                    client_ev.events = EPOLLIN;
                    client_ev.data.fd = new_fd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, new_fd, &client_ev);
                }
            }
            else if (event_buffer[i].events & EPOLLIN) {
                ssize_t bytes = recv(current_fd, buffer, sizeof(buffer), MSG_DONTWAIT);
                if (bytes > 0)
                {
                    client[current_fd].appendtoRecvBuf(buffer);
                    if (client[current_fd].getRecvBuf().size() > 4096)
                    {
                        Server::handeleDisconnect(current_fd);
                        continue;
                    }
                    // check if message full "\r\n"
                    while ((pos = client[current_fd].getRecvBuf().find("\r\n")) != std::string::npos)
                    {
                        std::string line = client[current_fd].getRecvBuf().substr(0, pos);
                        client[current_fd].getRecvBuf().erase(0, pos + 2);
                        if (line.size() > 510)
                        {
                            Server::handeleDisconnect(current_fd);
                            break;
                        }
                        Command msg = Parser.parse(line);
                        // message is ready to go to dispatcher
                    }
                }
                else if (bytes == 0)
                    Server::handeleDisconnect(current_fd);
                else {
                    if (errno != EAGAIN || errno != EWOULDBLOCK)  // i think it should be && here not ||
                        Server::handeleDisconnect(current_fd);
                }
            }
        }
    }
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

const std::string& Server::getServerName() const {
    return (serverName);
}
