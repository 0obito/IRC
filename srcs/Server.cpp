#include "../includes/Server.hpp"
#include "../includes/Parser.hpp"
#include "../includes/Dispatcher.hpp"
#include <cerrno>
#include <cstdlib>

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
    // clientMap[new_fd] = Client(new_fd);                                                                          // not the correct way of using map
    clientMap.insert(std::make_pair(new_fd, Client(new_fd)));
    return (new_fd);
}

void    Server::handeleDisconnect(int fd)
{
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
    clientMap.erase(fd);
}

ssize_t Server::send_message(int fd, std::string &buf){
    ssize_t send_size;

    if (buf.empty())
        return(0);
    send_size = send(fd, buf.c_str(), buf.size(), MSG_DONTWAIT);
    if (send_size > 0){
        buf.erase(0, send_size);
    }
    else if (send_size == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
        Server::handeleDisconnect(fd);
    }
    return (send_size);
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
        std::cerr << "[ERROR] Creating an epoll instance has failed!" << std::endl;
        exit(1);
    }
    ev.events = EPOLLIN;
    ev.data.fd = serversocket;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serversocket, &ev);
    while(true) {
        nfds = epoll_wait(epfd, event_buffer, MAX_EVENTS, -1);
        std::cout << "fds: " << nfds << std::endl;
        for (int i = 0; i < nfds; i++) {
            current_fd = event_buffer[i].data.fd;
            std::cout << "current fd: " << current_fd << std::endl;
            std::map<int, Client>::iterator iter = clientMap.find(current_fd);
            if (event_buffer[i].events & (EPOLLERR | EPOLLHUP)) {
                Server::handeleDisconnect(current_fd);
            }
            else if (current_fd == serversocket) {
                new_fd = Server::acceptNewClient();
                std::cout << "new client: " << new_fd << std::endl;
                if (new_fd != -1) {
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
                    if (iter == clientMap.end()) {
                        std::cerr << "[ERROR] Couldn't find client with file descriptor number " << current_fd << std::endl;
                        // [?] need to handle error here (close the client or whatever)
                    }
                    /* std::string &r_buf = clientMap[current_fd].getRecvBuf(); */                                  // not the correct way of using map
                    std::string &r_buf = iter->second.getRecvBuf();
                    r_buf.append(buffer, bytes);
                    if (r_buf.size() > 4096)
                    {
                        Server::handeleDisconnect(current_fd);
                        continue;
                    }
                    // check if message full "\r\n"
                    while ((position = r_buf.find("\r\n")) != std::string::npos)
                    {
                        std::string line = r_buf.substr(0, position);
                        r_buf.erase(0, position + 2);
                        if (line.size() > 510)
                        {
                            Server::handeleDisconnect(current_fd);
                            break;
                        }
                        Command msg = Parser::parse(line);
                        commandDispatcher cmdDispatcher;
                        // cmdDispatcher.routeCommand(*this, clientMap[current_fd], msg);                           // not the correct way of using map
                        cmdDispatcher.routeCommand(*this, iter->second, msg);
                    }
                    // std::string &sendQueue = clientMap[current_fd].getSendQueue();                               // not the correct way of using map
                    std::string &sendQueue = iter->second.getSendQueue();
                    if (!sendQueue.empty()){
                        if (Server::send_message(current_fd, sendQueue) != (ssize_t)sendQueue.size()){
                            struct epoll_event current_ev; 
                            current_ev.events = EPOLLIN | EPOLLOUT;
                            current_ev.data.fd = current_fd;
                            epoll_ctl(epfd, EPOLL_CTL_MOD, current_fd, &current_ev);
                        }
                    }
                }
                else if (bytes == 0) {
                    Server::handeleDisconnect(current_fd);
                }
                else {
                    if (errno != EAGAIN && errno != EWOULDBLOCK)
                        Server::handeleDisconnect(current_fd);
                }
            }
            else if (event_buffer[i].events & EPOLLOUT) {
                if (iter == clientMap.end()) {
                    std::cerr << "[ERROR] Couldn't find client with file descriptor number " << current_fd << std::endl;
                    // [?] need to handle error here (close the client or whatever)
                }
                // [?] Shouldn't we check the return of send_message() down here. In case it fails? 
                // Server::send_message(current_fd, clientMap[current_fd].getSendQueue());                          // not the correct way of using map
                Server::send_message(current_fd, iter->second.getSendQueue());
                // if (clientMap[current_fd].getSendQueue().empty()) {                                              // not the correct way of using map
                if (iter->second.getSendQueue().empty()) {
                    struct epoll_event current_ev;
                    current_ev.events = EPOLLIN;
                    current_ev.data.fd = current_fd;
                    epoll_ctl(epfd, EPOLL_CTL_MOD, current_fd, &current_ev);
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
    multiplexar();
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

int Server::isNicknameTaken(const std::string& nickname) const {
    std::map<int, Client>::iterator it;

    for(it = clientMap.begin(); it != clientMap.end(); it++){
        if (it->second.getNick() == nickname)
            return(it->second.getFd());
    }
    return(-1);
}
