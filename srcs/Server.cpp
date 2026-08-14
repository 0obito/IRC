#include "../includes/Server.hpp"

#include "../includes/Client.hpp"
#include "../includes/Parser.hpp"
#include "../includes/Channel.hpp"
#include "../includes/Dispatcher.hpp"
#include "../includes/Utils.hpp"

int signalStatus = 0;

// canonical form
Server::Server(int port_in, const std::string& pwd) 
    :port(port_in),
    password(pwd),
    serverName("ircDyalna"), 
    serverSocket(-1),
    epfd(-1)
{}

Server::~Server() {
    if (serverSocket != -1) {
        close(serverSocket);
        serverSocket = -1;
    }
    disconnectServer();
}


// getters
const std::string&                      Server::getServerName() const {
    return (serverName);
}

const std::string&                      Server::getPassword() const {
    return(password);
}

std::map<std::string, Channel*>&        Server::getChannels() {
    return chansMap;
}

int                                     Server::getEPFD() const {
    return epfd;
}

std::map<int, Client>&                  Server::getMap() {
    return clientMap;
}

Channel*                                Server::getChannel(const std::string& name) {
    std::map<std::string, Channel*>::iterator it = chansMap.find(name);
    if (it != chansMap.end())
        return it->second;
    return NULL;
}

int                                     Server::nicknameOwner(std::string& nickname) {
    std::map<int, Client>::iterator it;

    for(it = clientMap.begin(); it != clientMap.end(); it++){
        if (it->second.getNickOk() && it->second.getNick() == nickname)
            return(it->second.getFd());
    }
    return(-1);
}


int Server::acceptNewClient() {
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    int newFd = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
    // only process if accept was successful, obviously
    if (newFd != -1) {
        // safe insert into map
        clientMap.insert(std::make_pair(newFd, Client(newFd)));

        // get client ip, add it to the object
        std::string ip = inet_ntoa(clientAddr.sin_addr);
        std::map<int, Client>::iterator it = clientMap.find(newFd);
        if (it != clientMap.end()) {
            it->second.setHostname(ip);
        }
    }
    return newFd;
}


void            Server::addChannel(Channel* channel) {
    chansMap[channel->getName()] = channel;
}

void            Server::removeChannel(const std::string& name) {
    std::string lowerName = toLower(name);
    std::map<std::string, Channel*>::iterator it = chansMap.find(lowerName);
    if (it != chansMap.end()) {
        chansMap.erase(it);
    }
}


// server backbone
void Server::disconnectServer() {
    std::map<int, Client>::iterator clIt = clientMap.begin();
    while (clIt != clientMap.end()) {
        disconnectClient(clIt++);
    }

    std::map<std::string, Channel*>::iterator chIt = chansMap.begin();
    while (chIt != chansMap.end()) {
        delete chIt->second;
        ++chIt;
    }
    chansMap.clear();

    if (epfd != -1) {
        close(epfd);
        epfd = -1;
    }
}

void            Server::initServer()
{
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;
    serverSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (serverSocket == -1) {
        throw std::runtime_error("Failed to create server socket");
    }
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        throw std::runtime_error("Failed to set socket options");
    }
    if (bind(serverSocket, (sockaddr*)&address, sizeof(address)) == -1) {
        throw std::runtime_error("Failed to bind to port (is it already in use?)");
    }
    if (listen(serverSocket, SOMAXCONN) == -1) {
        throw std::runtime_error("Failed to listen on socket");
    }
}

void            Server::multiplexer() {
    char buffer[1024];
    int nfds;
    int currentFd;
    size_t position;
    struct epoll_event ev;

    epfd = epoll_create1(0);
    if (epfd < 0) {
        throw std::runtime_error("Creating epoll instance failed");
    }

    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.fd = serverSocket;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serverSocket, &ev);

    commandDispatcher cmdDispatcher;

    while (signalStatus == 0) {
        nfds = epoll_wait(epfd, eventBuffer, MAX_EVENTS, 1000);

        for (int i = 0; i < nfds; i++) {
            currentFd = eventBuffer[i].data.fd;

            // 1. THE LISTENER (New Connections)
            if (currentFd == serverSocket) {
                int newFd = acceptNewClient();
                if (newFd != -1) {
                    struct epoll_event client_ev;
                    memset(&client_ev, 0, sizeof(client_ev));
                    client_ev.events = EPOLLIN;
                    client_ev.data.fd = newFd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, newFd, &client_ev);
                }
                continue;
            }

            // 2. THE VALIDATOR
            // At this point, currentFd MUST be a client. We safely find them.
            std::map<int, Client>::iterator iter = clientMap.find(currentFd);
            if (iter == clientMap.end()) {
                // ghost FD
                epoll_ctl(epfd, EPOLL_CTL_DEL, currentFd, NULL);
                close(currentFd);
                continue;
            }
            std::cout << "NICK: " << iter->second.getNick() << std::endl;

            // 3. THE REAPER (Errors & Hang-ups)
            if (eventBuffer[i].events & (EPOLLERR | EPOLLHUP)) {
                disconnectClient(iter);
                continue;
            }

            // 4. THE READER (EPOLLIN)
            if (eventBuffer[i].events & EPOLLIN) {
                ssize_t bytes = recv(currentFd, buffer, sizeof(buffer), MSG_DONTWAIT);

                if (bytes == 0) {
                    disconnectClient(iter);
                    continue; // Client explicitly disconnected
                }
                else if (bytes < 0) {
                    if (errno != EAGAIN && errno != EWOULDBLOCK) {
                        disconnectClient(iter); // A real socket error occurred
                    }
                    continue; // Nothing to read, or error handled
                }

                // data received well, process it
                iter->second.updateActivity();
                std::string &r_buf = iter->second.getRecvBuf();
                r_buf.append(buffer, bytes);

                // protect against data overload and malicious stuff
                if (r_buf.size() > 4096) {
                    disconnectClient(iter);
                    continue;
                }

                bool parsingFailed = false;
                while ((position = r_buf.find("\r\n")) != std::string::npos) {
                    std::string line = r_buf.substr(0, position);
                    r_buf.erase(0, position + 2);

                    if (line.size() > 510) {
                        disconnectClient(iter);
                        parsingFailed = true;
                        break; 
                    }

                    Command msg = Parser::parse(line);
                    cmdDispatcher.routeCommand(*this, iter->second, msg);
                }

                // parser disconnected the client, we can just skip EPOLLOUT
                if (parsingFailed) {
                    continue;
                }

                // If the command handlers queued up responses, awaken the Writer
                if (!iter->second.getSendQueue().empty()) {
                    struct epoll_event current_ev;
                    memset(&current_ev, 0, sizeof(current_ev));
                    current_ev.events = EPOLLIN | EPOLLOUT;
                    current_ev.data.fd = currentFd;
                    epoll_ctl(epfd, EPOLL_CTL_MOD, currentFd, &current_ev);
                }
            }

            // 5. THE WRITER (EPOLLOUT)
            // We re-find the iterator in case the Reader deleted it to prevent segfaults
            iter = clientMap.find(currentFd);
            if (iter != clientMap.end() && (eventBuffer[i].events & EPOLLOUT)) {
                ssize_t sentChunk = sendMessage(currentFd, iter->second.getSendQueue());

                // an error occured when trying to send buffer content
                if (sentChunk == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
                    disconnectClient(iter);
                    continue;
                }

                // we successfully sent the whole buffer
                if (iter->second.getSendQueue().empty()) {
                    struct epoll_event current_ev;
                    memset(&current_ev, 0, sizeof(current_ev));
                    current_ev.events = EPOLLIN;
                    current_ev.data.fd = currentFd;
                    epoll_ctl(epfd, EPOLL_CTL_MOD, currentFd, &current_ev);

                    // If the client was flagged for death (e.g., /QUIT), kill them AFTER sending the final message
                    if (iter->second.isDead()) {
                        disconnectClient(currentFd);
                    }
                }
            }
        }
        pingPong();
    }
}


// other
void            Server::disconnectClient(int fd)
{
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
    clientMap.erase(fd);
    // ISSUE. loop through joined channels and disconnect, or broadcast a msg or smthg
}

void            Server::disconnectClient(std::map<int, Client>::iterator it)   // I newly added this, it's faster but we can't use it all the time
{
    epoll_ctl(epfd, EPOLL_CTL_DEL, it->first, NULL);
    close(it->first);
    clientMap.erase(it);
    // ISSUE. loop through joined channels and disconnect, or broadcast a msg or smthg
}

ssize_t         Server::sendMessage(int fd, std::string &buf) {
    ssize_t send_size;

    // empty buffer, nothing to send
    if (buf.empty())
        return(0);

    // attempt to send buffer content
    send_size = send(fd, buf.c_str(), buf.size(), MSG_DONTWAIT);

    // buffer content or some of it was sent
    if (send_size > 0) {
        buf.erase(0, send_size);
    }

    return (send_size);
}

void            Server::pingPong() {
    static time_t lastSweep = time(NULL);
    time_t now = time(NULL);
    if (now - lastSweep >= 60) {
        lastSweep = now;
        for (std::map<int, Client>::iterator it = clientMap.begin(); it != clientMap.end(); ++it) {
            Client& cl = it->second;
            int idleTime = now - cl.getLastActivity();
            if (idleTime > 60) {
                int fd = cl.getFd();
                if (!cl.isWaitingForPong()) {
                    std::string pingMsg = "PING :keepalive\r\n";
                    cl.getSendQueue() += pingMsg;
                    cl.setWaitingForPong(true);
                    struct epoll_event current_ev;
                    memset(&current_ev, 0, sizeof(current_ev));
                    current_ev.events = EPOLLOUT | EPOLLIN;
                    current_ev.data.fd = fd;
                    epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &current_ev);
                }
                else {
                    std::string clNick = cl.getNick().empty() ? "*" : cl.getNick();
                    std::string clUser = cl.getUser().empty() ? "*" : cl.getUser();
                    std::string clHost = cl.getHostname();
                    std::string quitMsg = ":" + clNick + "!" + clUser + "@" + clHost + " QUIT :Ping timeout: 120 seconds\r\n";
                    cl.getSendQueue() += quitMsg;
                    cl.setDead(true);
                    struct epoll_event current_ev;
                    memset(&current_ev, 0, sizeof(current_ev));
                    current_ev.events = EPOLLOUT;
                    current_ev.data.fd = cl.getFd();
                    epoll_ctl(epfd, EPOLL_CTL_MOD, cl.getFd(), &current_ev);
                }
            }
        }
    }
}
