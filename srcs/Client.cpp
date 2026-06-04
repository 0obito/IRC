#include "../includes/Client.hpp"
#include "../includes/Utils.hpp"
#include <ctime>

Client::Client(int fd)
    : fd(fd),
      passOk(false),
      nickOk(false),
      userOk(false),
      registerOk(false),
      waitingForPong(false)
{
    lastActivity = time(NULL);
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

void Client::setRegistered(bool value) {
    registerOk = value;
}

bool Client::getPassOk() const{
    return (passOk);
}

bool Client::getNickOk() const{
    return (nickOk);
}

bool Client::getUserOk() const{
    return (userOk);
}

bool Client::isRegistered() const
{
    return registerOk;
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

void Client::updateActivity() {
    lastActivity = time(NULL);
    waitingForPong = false;
}

time_t Client::getLastActivity() const {
    return lastActivity;
}

bool Client::isWaitingForPong() const {
    return waitingForPong;
}

void Client::setWaitingForPong(bool status) {
    waitingForPong = status;
}
