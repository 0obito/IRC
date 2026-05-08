#include "Client.hpp"
#include "Utils.hpp"

Client::Client(int fd)
    : fd(fd),
      passOk(false),
      nickOk(false),
      userOk(false)
{
}

Client::~Client() {}

const std::string &Client::getNick() const {
    return nick;
}

const std::string &Client::getUser() const {
    return user;
}

const std::string &Client::getRealname() const
{
    return realname;
}

const std::string &Client::getHostname() const
{
    return hostname;
}

int Client::getFd() const {
    return fd;
}

void Client::setNick(const std::string &n) {
// [obito] Since we'll have to send a numeric reply if setting a nickname fails,
//          it makes more sense to check the validity of the nickname in my Dispatcher.
//         So I check the validity in my Dispatcher, if things are good I call your setter,
//          if not then I return a convenient numeric reply.
//         Also, I changed the 2nd if statement. Correct me if I'm wrong, but I think it's != instead of ==

/*  if (n.empty())
        return ;

    if (n.find(' ') != std::string::npos || n.find(',') != std::string::npos || n.find('*') != std::string::npos || n.find('.') != std::string::npos
        || n.find('?') != std::string::npos || n.find('!') != std::string::npos || n.find('@') != std::string::npos)
        return ;

    if (n[0] == '$' || n[0] == ':' || n[0] == '#' || n[0] == '&'
        || n[0] == '~' || n[0] == '%' || n[0] == '+')
        return ;    */

    this->nick = n;
}

void Client::setUser(const std::string &u) {
    user = u;
}

void Client::setRealname(const std::string &r) {
    realname = r;
}

void Client::setHostname(const std::string &h) {
    hostname = h;
}

void Client::setPassOk(bool value) {
    passOk = value;
}

void Client::setNickOk(bool value) {
    nickOk = value;
}

void Client::setUserOk(bool value) {
    userOk = value;
}

// [obito] I added the following getters to know the state of some booleans:
//      getPassOk()
//      getNickOk()
//      getUserOk()

bool Client::getPassOk() const{
    return (passOk);    // PassOk getter
}

bool Client::getNickOk() const{
    return (nickOk);    // NickOk getter
}

bool Client::getUserOk() const{
    return (userOk);    // UserOk getter
}

bool Client::isRegistered() const
{
    return passOk && nickOk && userOk;
}

std::string &Client::getRecvBuf() {
    return recvBuf;
}
std::string &Client::getSendQueue() {
    return sendQueue;
}

void Client::joinChannel(const std::string &name)
{
    channels.insert(toLower(name));
}

void Client::leaveChannel(const std::string &name)
{
    channels.erase(toLower(name));
}

bool Client::isInChannel(const std::string &name) const
{
    return channels.find(toLower(name)) != channels.end();
}