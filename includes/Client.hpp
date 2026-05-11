#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <set>

/*
 * Client class
 * It represents one connected user.
 * It stores all information about the client:
 * fd, nickname, username, buffers, and registration state.
 * It only holds data, the server handles the logic.
 */

class Client
{
private:
    int fd;

    std::string nick;
    std::string user;
    std::string realname;
    std::string hostname;

    bool passOk;
    bool nickOk;
    bool userOk;
    bool registerOk;

    std::string recvBuf;
    std::string sendQueue;

    std::set<std::string> channels;

public:
    Client(int fd);
    ~Client();

    // identity getters
    const std::string &getNick() const;
    const std::string &getUser() const;
    const std::string &getRealname() const;
    const std::string &getHostname() const;
    const int getFd() const;

    // identity setters
    void setNick(const std::string &n);
    void setUser(const std::string &u);
    void setRealname(const std::string &r);
    void setHostname(const std::string &h);

    // state setters
    void setPassOk(bool value);
    void setNickOk(bool value);
    void setUserOk(bool value);
    void setRegistered(bool value);

    // state getters
    const bool getPassOk() const;
    const bool getNickOk() const;
    const bool getUserOk() const;
    const bool isRegistered() const;

    // buffers
    void        appendtoRecvBuf(std::string buf);
    std::string &getRecvBuf();
    std::string &getSendQueue();

    // channels
    void joinChannel(const std::string &name);
    void leaveChannel(const std::string &name);
    bool isInChannel(const std::string &name) const;
};

#endif