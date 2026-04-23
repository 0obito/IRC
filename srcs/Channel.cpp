#include "Channel.hpp"
#include "Utils.hpp"

Channel::Channel(const std::string &n)
    : name(toLower(n)),
      topic(""),
      key(""),
      userLimit(0),
      inviteOnly(false),
      topicRestricted(false)
{
}

Channel::~Channel()
{
}

const std::string &Channel::getName() const {
    return name;
}
const std::string &Channel::getTopic() const {
    return topic;
}

void Channel::setTopic(const std::string &t)
{
    topic = t;
}

void Channel::setInviteOnly(bool value)
{
    inviteOnly = value;
}

bool Channel::isInviteOnly() const
{
    return inviteOnly;
}

void Channel::setTopicRestricted(bool value)
{
    topicRestricted = value;
}

bool Channel::isTopicRestricted() const
{
    return topicRestricted;
}

void Channel::setKey(const std::string &k)
{
    key = k;
}

const std::string &Channel::getKey() const
{
    return key;
}

void Channel::setUserLimit(int limit)
{
    userLimit = limit;
}

bool Channel::isFull() const
{
    if (userLimit <= 0)
        return false;

    return memberFds.size() >= (size_t)userLimit;
}

void Channel::addMember(int fd)
{
    memberFds.insert(fd);
}

void Channel::removeMember(int fd)
{
    memberFds.erase(fd);
    operatorFds.erase(fd);
    inviteList.erase(fd);
}

bool Channel::isMember(int fd) const
{
    return memberFds.find(fd) != memberFds.end();
}

const std::set<int> &Channel::getMembers() const
{
    return memberFds;
}

void Channel::addOperator(int fd)
{
    if (isMember(fd))
        operatorFds.insert(fd);
}

void Channel::removeOperator(int fd)
{
    operatorFds.erase(fd);
}

bool Channel::isOperator(int fd) const
{
    return operatorFds.find(fd) != operatorFds.end();
}

void Channel::invite(int fd)
{
    inviteList.insert(fd);
}

bool Channel::isInvited(int fd) const
{
    return inviteList.find(fd) != inviteList.end();
}

void Channel::removeInvite(int fd)
{
    inviteList.erase(fd);
}
