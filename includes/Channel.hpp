#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>

/*
 * Channel class
 * Represents an IRC channel.
 * Stores channel state: members, operators, modes, etc.
 * Provides functions to manage users and channel rules.
 */

class Channel
{
private:
    std::string name;
    std::string topic;
    std::string key;

    int userLimit;

    bool inviteOnly;
    bool topicRestricted;

    std::set<int> memberFds;
    std::set<int> operatorFds;
    std::set<int> inviteList;

public:
    Channel(const std::string &name);
    ~Channel();

    // 🔹 basic info
    const std::string &getName() const;
    const std::string &getTopic() const;
    void setTopic(const std::string &t);

    // 🔹 modes
    void setInviteOnly(bool value);
    bool isInviteOnly() const;

    void setTopicRestricted(bool value);
    bool isTopicRestricted() const;

    void setKey(const std::string &k);
    const std::string &getKey() const;

    void setUserLimit(int limit);
    bool isFull() const;

    // 🔹 members
    void addMember(int fd);
    void removeMember(int fd);
    bool isMember(int fd) const;

    const std::set<int> &getMembers() const;

    // 🔹 operators
    void addOperator(int fd);
    void removeOperator(int fd);
    bool isOperator(int fd) const;

    // 🔹 invite system
    void invite(int fd);
    bool isInvited(int fd) const;
    void removeInvite(int fd);
};

#endif
