#include "../includes/Dispatcher.hpp"
#include "../includes/DispatcherUtils.hpp"
#include "../includes/Server.hpp"
#include "../includes/Utils.hpp"
#include "../includes/Channel.hpp"
#include <string.h>
#include <sstream>

// an example for a welcoming sequence, might change a thing or two later
// i am not sure if this could've been built using makeReply(), I built it before checking the method :-)
void welcomingSeq(Client& client, const std::string serverName) {
    std::string nick = client.getNick();
    std::string user = client.getUser();
    std::string version = "0.1";
    std::stringstream ss;

    ss << ":" << serverName << " 001 " << nick;
    ss << " :Welcome to our Internet Relay Network " << nick << "!" << user << "@127.0.0.1\r\n";

    ss << ":" << serverName << " 002 " << nick;
    ss << " :Your host is " << serverName << ", running version " << version << "\r\n";

    ss << ":" << serverName << " 003 " << nick;
    ss << " :This server was created f 3am lfil\r\n";

    ss << ":" << serverName << " 004 " << nick;
    ss << " " << serverName << " " << version << " io itkol\r\n";

    ss << ":" << serverName << " 005 " << nick;
    ss << " CHANTYPES=# CHANNELLEN=32 NICKLEN=9 NETWORK=OurNetwork :are supported by this server\r\n";

    std::string finalMsg = ss.str();
    client.getSendQueue() += finalMsg;
    return ;
}

void registerClient(Client& client, const std::string serverName) {
    if (!client.isRegistered()) {
        if (client.getPassOk() && client.getNickOk() && client.getUserOk()) {
            client.setRegistered(true);
            welcomingSeq(client, serverName);
        }
    }
    return ;
}

void handleCAP(Server& server, Client& client, Command& parsedMsg) {
    (void)server;
    (void)client;
    (void)parsedMsg;
    // std::string reply = ":" + server.getServerName() + " CAP * LS :\r\n";
    // client.getSendQueue() += reply;
    // std::cout << "CAP * LS :" << std::endl;
}

void handlePASS(Server& server, Client& client, Command& parsedMsg) {
    std::string targetNick = client.getNick().empty() ? "*" : client.getNick();
    std::string serverName = server.getServerName();
    std::string reply;

    if (client.isRegistered()) {
        reply = makeReply(serverName, 462, targetNick, "Unauthorized command (already registered)");
        client.getSendQueue() += reply;
        // std::cout << "ERR_ALREADYREGISTERED (462)" << std::endl;
        return ;
    }
    if (parsedMsg.params.empty()) {
        reply = makeReply(serverName, 461, targetNick, "Not enough parameters", parsedMsg.command);
        client.getSendQueue() += reply;
        // std::cout << "ERR_NEEDMOREPARAMS (461)" << std::endl;
        return ;
    }
    if (parsedMsg.params[0] == server.getPassword()) {
        client.setPassOk(true);
        return ;
    }
    else {
        client.setPassOk(false);
        reply = makeReply(serverName, 464, targetNick, "Password incorrect");
        client.getSendQueue() += reply;
        // std::cout << "ERR_PASSWDMISMATCH (464)" << std::endl;
        return ;
    }
}

bool nickIsValid(const std::string &nickName) {
    if (nickName.empty())
        return false;
    if (nickName.find(' ') != std::string::npos || nickName.find(',') != std::string::npos || nickName.find('*') != std::string::npos || nickName.find('.') != std::string::npos
        || nickName.find('?') != std::string::npos || nickName.find('!') != std::string::npos || nickName.find('@') != std::string::npos)
        return false;
    if (nickName[0] == '$' || nickName[0] == ':' || nickName[0] == '#' || nickName[0] == '&'
        || nickName[0] == '~' || nickName[0] == '%' || nickName[0] == '+')
        return false;
    return true;
}

void handleNICK(Server& server, Client& client, Command& parsedMsg) {
    std::string targetNick = client.getNick().empty() ? "*" : client.getNick();
    std::string serverName = server.getServerName();
    std::string reply;

    if (!client.getPassOk()) {
        reply = makeReply(serverName, 464, targetNick, "Password incorrect");
        client.getSendQueue() += reply;
        // std::cout << "ERR_PASSWDMISMATCH (464) | Password was not supplied" << std::endl;
        return ;
    }
    if (parsedMsg.params.empty()) {
        reply = makeReply(serverName, 431, targetNick, "No nickname given");
        client.getSendQueue() += reply;
        // std::cout << "ERR_NONICKNAMEGIVEN (431)" << std::endl;
        return ;
    }
    if (parsedMsg.params.size() > 1) {
        //  nick has more than 1 param
        reply = makeReply(serverName, 461, targetNick, "Syntax error", parsedMsg.command);
        client.getSendQueue() += reply;
        // std::cout << "ERR_NEEDMOREPARAMS (461)" << std::endl;
        return ;
    }
    std::string nickName = parsedMsg.params[0];
    if (client.getNickOk() && client.getNick() == nickName) {
        return ;
    }
    if (server.nicknameOwner(nickName) != -1) {
        reply = makeReply(serverName, 433, targetNick, "Nickname is already in use", nickName);
        client.getSendQueue() += reply;
        // std::cout << "ERR_NICKNAMEINUSE (433)" << std::endl;
        return ;
    }
    if (!nickIsValid(nickName)) {
        reply = makeReply(serverName, 432, targetNick, "Erroneous nickname", nickName);
        client.getSendQueue() += reply;
        // std::cout << "ERR_ERRONEUSNICKNAME (432)" << std::endl;
        return ;
    }
    client.setNick(nickName);
    client.setNickOk(true);
    registerClient(client, serverName);
    return ;
}

void handleUSER(Server& server, Client& client, Command& parsedMsg) {
    std::string targetNick = client.getNick().empty() ? "*" : client.getNick();
    std::string serverName = server.getServerName();
    std::string reply;

    if (client.isRegistered()) {
        reply = makeReply(serverName, 462, targetNick, "Unauthorized command (already registered)");
        client.getSendQueue() += reply;
        // std::cout << "ERR_ALREADYREGISTERED (462)" << std::endl;
        return ;
    }
    if (!client.getPassOk()) {
        reply = makeReply(serverName, 464, targetNick, "Password incorrect");
        client.getSendQueue() += reply;
        // std::cout << "ERR_PASSWDMISMATCH (464) | Password was not supplied" << std::endl;
        return ;
    }
    if (parsedMsg.params.empty() || parsedMsg.params.size() < 4) {
        reply = makeReply(serverName, 461, targetNick, "Not enough parameters", parsedMsg.command);
        client.getSendQueue() += reply;
        // std::cout << "ERR_NEEDMOREPARAMS (461)" << std::endl;
        return ;
    }
    client.setUser(parsedMsg.params[0]);
    client.setRealname(parsedMsg.params[3]);
    client.setUserOk(true);
    registerClient(client, serverName);
}

void handlePRIVMSG(Server& server, Client& client, Command& parsedMsg) {
    std::string senderNick = client.getNick().empty() ? "*" : client.getNick();
    std::string serverName = server.getServerName();
    std::string reply;

    if (!client.isRegistered()) {
        reply = makeReply(serverName, 451, senderNick, "Connection not registered");
        client.getSendQueue() += reply;
        // std::cout << "ERR_NOTREGISTERED (451)" << std::endl;
        return ;
    }
    if (parsedMsg.params.empty()) {
        reply = makeReply(serverName, 411, senderNick, "No recipient given (PRIVMSG)");
        client.getSendQueue() += reply;
        // std::cout << "ERR_NORECIPIENT (411)" << std::endl;
        return ;
    }
    if (parsedMsg.params.size() < 2) {
        reply = makeReply(serverName, 412, senderNick, "No text to send");
        client.getSendQueue() += reply;
        // std::cout << "ERR_NOTEXTTOSEND (412)" << std::endl;
        return ;
    }
    if (parsedMsg.params.size() > 2) {
        reply = makeReply(serverName, 461, senderNick, "Syntax error", parsedMsg.command);
        client.getSendQueue() += reply;
        // std::cout << "ERR_NEEDMOREPARAMS (461)" << std::endl;
        return ;
    }

    std::string rawTargets = parsedMsg.params[0];
    std::string messageText = parsedMsg.params[1];
    std::stringstream ss(rawTargets);
    std::string singleTarget;

    while (std::getline(ss, singleTarget, ',')) {
        // empty name, go to the next one
        if (singleTarget.empty()) {
            continue ;
        }
        // it's to a channel, broadcast the message
        else if (singleTarget[0] == '#' || singleTarget[0] == '&') {
            broadcastToChannel(server, client, parsedMsg, singleTarget, messageText);
        }
        // it's to a user, send the message
        else {
            sendToClient(server, client, parsedMsg, singleTarget, messageText);
        }
    }
}

void handleNOTICE(Server& server, Client& client, Command& parsedMsg) {
    if (!client.isRegistered() || parsedMsg.params.size() != 2) {
        return ;
    }
    std::string rawTargets = parsedMsg.params[0];
    std::string messageText = parsedMsg.params[1];
    std::stringstream ss(rawTargets);
    std::string singleTarget;
    while (std::getline(ss, singleTarget, ',')) {
        int targetFD = server.nicknameOwner(singleTarget);
        if (targetFD == -1) {
            continue;
        }
        std::map<int, Client>::iterator iter = server.getMap().find(targetFD);
        if (iter != server.getMap().end()) {
            std::string reply = ":" + client.getNick() + "!" + client.getUser() + "@127.0.0.1 NOTICE " 
                    + iter->second.getNick() + " :" + messageText + "\r\n";
            iter->second.getSendQueue() += reply;
            struct epoll_event current_ev;
            memset(&current_ev, 0, sizeof(current_ev));
            current_ev.events = EPOLLOUT | EPOLLIN;
            current_ev.data.fd = iter->first;
            epoll_ctl(server.get_epfd(), EPOLL_CTL_MOD, iter->first, &current_ev);
        }
    }
}

void handlePING(Server& server, Client& client, Command& parsedMsg) {
    std::string targetNick = client.getNick().empty() ? "*" : client.getNick();
    std::string serverName = server.getServerName();
    std::string reply;

    if (!client.isRegistered()) {
        reply = makeReply(serverName, 451, targetNick, "Connection not registered");
        client.getSendQueue() += reply;
        // std::cout << "ERR_NOTREGISTERED (451)" << std::endl;
        return ;
    }
    if (parsedMsg.params.empty()) {
        reply = makeReply(serverName, 409, targetNick, "No origin specified");
        client.getSendQueue() += reply;
        // std::cout << "ERR_NOORIGIN (409)" << std::endl;
        return ;
    }
    if (parsedMsg.params.size() > 1 && parsedMsg.params[1] != serverName) {
        reply = makeReply(serverName, 402, targetNick, "No such server", parsedMsg.params[1]);
        client.getSendQueue() += reply;
        // std::cout << "ERR_NOSUCHSERVER (402)" << std::endl;
        return ;
    }

    std::stringstream ss;
    ss << ":" << serverName << " PONG " << serverName << " :" << parsedMsg.params[0] << "\r\n";
    reply = ss.str();
    client.getSendQueue() += reply;
    return ;
}

void handlePONG(Server& server, Client& client, Command& parsedMsg) {
    std::string targetNick = client.getNick().empty() ? "*" : client.getNick();
    std::string serverName = server.getServerName();
    std::string reply;

    if (!client.isRegistered()) {
        return ;
    }
    if (parsedMsg.params.empty()) {
        reply = makeReply(serverName, 409, targetNick, "No origin specified");
        client.getSendQueue() += reply;
        // std::cout << "ERR_NOORIGIN (409)" << std::endl;
        return ;
    }
    if (parsedMsg.params.size() > 2) {
        //  :irc.example.net 461 a pong :Syntax error
        reply = makeReply(serverName, 461, targetNick, "Syntax error", parsedMsg.command);
        client.getSendQueue() += reply;
        // std::cout << "ERR_NEEDMOREPARAMS (461)" << std::endl;
        return ;
    }
}

void handleJOIN(Server& server, Client& client, Command& parsedMsg) {
    std::string targetNick = client.getNick().empty() ? "*" : client.getNick();
    std::string serverName = server.getServerName();
    std::string reply;
    
    // Check if client is registered
    if (!client.isRegistered()) {
        reply = makeReply(serverName, 451, targetNick, "Connection not registered");
        client.getSendQueue() += reply;
        return;
    }
    
    // Check if enough parameters
    if (parsedMsg.params.empty()) {
        reply = makeReply(serverName, 461, targetNick, "Not enough parameters", parsedMsg.command);
        client.getSendQueue() += reply;
        return;
    }
    
    // Parse channel names (comma separated)
    std::string channelList = parsedMsg.params[0];
    std::string keyList = parsedMsg.params.size() > 1 ? parsedMsg.params[1] : "";
    std::stringstream channelStream(channelList);
    std::stringstream keyStream(keyList);
    std::string channelName, channelKey;
    
    while (std::getline(channelStream, channelName, ',')) {
        // Get key for this channel if provided
        if (!std::getline(keyStream, channelKey, ','))
            channelKey = "";
        
        // Convert channel name to lowercase for consistency
        std::string lowerChannelName = toLower(channelName);

        // Validate channel name (must start with # or &)
        if (lowerChannelName.empty() || (lowerChannelName[0] != '#' && lowerChannelName[0] != '&')) {
            reply = makeReply(serverName, 476, targetNick, "Invalid channel name", channelName);
            client.getSendQueue() += reply;
            continue;
        }

        // Check if channel exists
        Channel* channel = server.getChannel(lowerChannelName);
        bool isNewChannel = false;

        if (!channel) {
            // Create new channel
            channel = new Channel(lowerChannelName);
            server.addChannel(channel);
            isNewChannel = true;
        }

        // Check if already in channel
        if (channel->isMember(client.getFd())) {
            continue;
        }

        // Check invite-only
        if (channel->isInviteOnly() && !channel->isInvited(client.getFd())) {
            reply = makeReply(serverName, 473, targetNick, "Cannot join channel (+i)", channelName);
            client.getSendQueue() += reply;
            continue;
        }

        // Check key (if channel has a key and it doesn't match)
        if (!channel->getKey().empty() && channel->getKey() != channelKey) {
            reply = makeReply(serverName, 475, targetNick, "Cannot join channel (+k)", channelName);
            client.getSendQueue() += reply;
            continue;
        }

        // Check user limit
        if (channel->isFull()) {
            reply = makeReply(serverName, 471, targetNick, "Cannot join channel (+l)", channelName);
            client.getSendQueue() += reply;
            continue;
        }

        // Remove invite if present
        if (channel->isInvited(client.getFd()))
            channel->removeInvite(client.getFd());

        // Add member to channel
        channel->addMember(client.getFd());
        client.joinChannel(lowerChannelName);

        // If first user in channel, make them operator
        if (isNewChannel || channel->getMembers().size() == 1) {
            channel->addOperator(client.getFd());
        }

        // Send JOIN message to the client
        reply = ":" + client.getNick() + "!" + client.getUser() + "@localhost JOIN " + channelName + "\r\n";
        client.getSendQueue() += reply;

        // Send JOIN to all other members in channel
        const std::set<int>& members = channel->getMembers();
        for (std::set<int>::const_iterator it = members.begin(); it != members.end(); ++it) {
            if (*it != client.getFd()) {
                std::map<int, Client>::iterator iter = server.getMap().find(*it);
                if (iter != server.getMap().end()) {
                    iter->second.getSendQueue() += reply;

                    struct epoll_event current_ev;
                    memset(&current_ev, 0, sizeof(current_ev));
                    current_ev.events = EPOLLOUT | EPOLLIN;
                    current_ev.data.fd = *it;
                    epoll_ctl(server.get_epfd(), EPOLL_CTL_MOD, *it, &current_ev);
                }
            }
        }

        // Send topic (332 if exists, 331 if not)
        if (channel->getTopic().empty()) {
            reply = makeReply(serverName, 331, client.getNick(), "No topic is set", channelName);
        } else {
            reply = makeReply(serverName, 332, client.getNick(), channel->getTopic(), channelName);
        }
        client.getSendQueue() += reply;

        // Build NAMREPLY (353)
        std::stringstream namelist;
        for (std::set<int>::const_iterator it = members.begin(); it != members.end(); ++it) {
            if (it != members.begin())
                namelist << " ";

            // Find client to get nickname
            std::map<int, Client>::const_iterator clientIt = server.getMap().find(*it);
            if (clientIt != server.getMap().end()) {
                if (channel->isOperator(*it))
                    namelist << "@";
                namelist << clientIt->second.getNick();
            }
        }

        reply = makeReply(serverName, 353, client.getNick(), "= " + channelName + " :" + namelist.str());
        client.getSendQueue() += reply;

        // Send ENDOFNAMES (366)
        reply = makeReply(serverName, 366, client.getNick(), "End of /NAMES list", channelName);
        client.getSendQueue() += reply;
    }
}

void handlePART(Server& server, Client& client, Command& parsedMsg) {
    std::string targetNick = client.getNick().empty() ? "*" : client.getNick();
    std::string serverName = server.getServerName();
    std::string reply;
    
    // Check if client is registered
    if (!client.isRegistered()) {
        reply = makeReply(serverName, 451, targetNick, "Connection not registered");
        client.getSendQueue() += reply;
        return;
    }

    // Check if enough parameters
    if (parsedMsg.params.empty()) {
        reply = makeReply(serverName, 461, targetNick, "Not enough parameters", parsedMsg.command);
        client.getSendQueue() += reply;
        return;
    }

    // Parse channel names (comma separated)
    std::string channelList = parsedMsg.params[0];
    std::string reason = parsedMsg.params.size() > 1 ? parsedMsg.params[1] : "";
    std::stringstream channelStream(channelList);
    std::string channelName;

    while (std::getline(channelStream, channelName, ',')) {
        // Convert channel name to lowercase for consistency
        std::string lowerChannelName = toLower(channelName);

        // Validate channel name
        if (lowerChannelName.empty() || (lowerChannelName[0] != '#' && lowerChannelName[0] != '&')) {
            reply = makeReply(serverName, 403, targetNick, "No such channel", channelName);
            client.getSendQueue() += reply;
            continue;
        }
        
        // Check if channel exists
        Channel* channel = server.getChannel(lowerChannelName);
        if (!channel) {
            reply = makeReply(serverName, 403, targetNick, "No such channel", channelName);
            client.getSendQueue() += reply;
            continue;
        }

        // Check if client is in the channel
        if (!channel->isMember(client.getFd())) {
            reply = makeReply(serverName, 442, targetNick, "You're not on that channel", channelName);
            client.getSendQueue() += reply;
            continue;
        }

        // Build PART message (broadcast to ALL members including the departing user)
        std::string partMsg;
        if (reason.empty()) {
            partMsg = ":" + client.getNick() + "!" + client.getUser() + "@localhost PART " + channelName + "\r\n";
        } else {
            partMsg = ":" + client.getNick() + "!" + client.getUser() + "@localhost PART " + channelName + " :" + reason + "\r\n";
        }

        // Send PART to all members in the channel (including the departing user)
        const std::set<int>& members = channel->getMembers();
        for (std::set<int>::const_iterator it = members.begin(); it != members.end(); ++it) {
            std::map<int, Client>::iterator iter = server.getMap().find(*it);
            if (iter != server.getMap().end()) {
                iter->second.getSendQueue() += partMsg;

                struct epoll_event current_ev;
                memset(&current_ev, 0, sizeof(current_ev));
                current_ev.events = EPOLLOUT | EPOLLIN;
                current_ev.data.fd = *it;
                epoll_ctl(server.get_epfd(), EPOLL_CTL_MOD, *it, &current_ev);
            }
        }

        // Remove client from channel
        channel->removeMember(client.getFd());
        client.leaveChannel(lowerChannelName);

        // Delete channel if empty
        if (channel->getMembers().empty()) {
            server.removeChannel(lowerChannelName);
            delete channel;
        }
    }
}

// Syntax: KICK #channel client [reason]
void handleKICK(Server& server, Client& client, Command& parsedMsg) {
    std::string targetNick = client.getNick().empty() ? "*" : client.getNick();
    std::string serverName = server.getServerName();
    std::string reply;
    
    // registered?
    if (!client.isRegistered()) {
        reply = makeReply(serverName, 451, targetNick, "Connection not registered");
        client.getSendQueue() += reply;
        return;
    }
    
    // enough parameters?
    if (parsedMsg.params.size() < 2) {
        reply = makeReply(serverName, 461, targetNick, "Not enough parameters", parsedMsg.command);
        client.getSendQueue() += reply;
        return;
    }
    
    std::string channelName = parsedMsg.params[0];
    std::string targetToKick = parsedMsg.params[1];
    std::string reason = parsedMsg.params.size() > 2 ? parsedMsg.params[2] : "No reason given";
    std::string lowerChannelName = toLower(channelName);
    
    // channel name s7i7?
    if (lowerChannelName.empty() || (lowerChannelName[0] != '#' && lowerChannelName[0] != '&')) {
        reply = makeReply(serverName, 403, targetNick, "No such channel", channelName);
        client.getSendQueue() += reply;
        return;
    }
    
    // channel exists?
    Channel* channel = server.getChannel(lowerChannelName);
    if (!channel) {
        reply = makeReply(serverName, 403, targetNick, "No such channel", channelName);
        client.getSendQueue() += reply;
        return;
    }
    
    // client is in the channel?
    if (!channel->isMember(client.getFd())) {
        reply = makeReply(serverName, 442, targetNick, "You're not on that channel", channelName);
        client.getSendQueue() += reply;
        return;
    }
    
    // client is a channel operator?
    if (!channel->isOperator(client.getFd())) {
        reply = makeReply(serverName, 482, targetNick, "You're not channel operator", channelName);
        client.getSendQueue() += reply;
        return;
    }
    
    // target client nickname kayn?
    int targetFd = server.nicknameOwner(targetToKick);
    if (targetFd == -1) {
        reply = makeReply(serverName, 401, targetNick, "No such nick/channel", targetToKick);
        client.getSendQueue() += reply;
        return;
    }
    
    // target is in the channel?
    if (!channel->isMember(targetFd)) {
        reply = makeReply(serverName, 441, targetNick, "They aren't on that channel", targetToKick + " " + channelName);
        client.getSendQueue() += reply;
        return;
    }
    
    // kick yourself? la asa7bi!
    if (targetFd == client.getFd()) {
        reply = makeReply(serverName, 482, targetNick, "You can't kick yourself", channelName);
        client.getSendQueue() += reply;
        return;
    }
    
    // build KICK message
    std::string kickMsg = ":" + client.getNick() + "!" + client.getUser() + "@localhost KICK " + channelName + " " + targetToKick + " :" + reason + "\r\n";
    
    // send KICK to all members in the channel
    const std::set<int>& members = channel->getMembers();
    for (std::set<int>::const_iterator it = members.begin(); it != members.end(); ++it) {
        std::map<int, Client>::iterator iter = server.getMap().find(*it);
        if (iter != server.getMap().end()) {
            iter->second.getSendQueue() += kickMsg;
            
            struct epoll_event current_ev;
            memset(&current_ev, 0, sizeof(current_ev));
            current_ev.events = EPOLLOUT | EPOLLIN;
            current_ev.data.fd = *it;
            epoll_ctl(server.get_epfd(), EPOLL_CTL_MOD, *it, &current_ev);
        }
    }
    
    // remove target from channel
    channel->removeMember(targetFd);
    
    // find target client and remove channel from their list
    std::map<int, Client>::iterator targetIter = server.getMap().find(targetFd);
    if (targetIter != server.getMap().end()) {
        targetIter->second.leaveChannel(lowerChannelName);
    }
    
    // delete channel if empty
    if (channel->getMembers().empty()) {
        server.removeChannel(lowerChannelName);
        delete channel;
    }
}

void handleTOPIC(Server& server, Client& client, Command& parsedMsg) {
    std::string targetNick = client.getNick().empty() ? "*" : client.getNick();
    std::string serverName = server.getServerName();
    std::string reply;
    
    // registered?
    if (!client.isRegistered()) {
        reply = makeReply(serverName, 451, targetNick, "Connection not registered");
        client.getSendQueue() += reply;
        return;
    }

    // enough parameters?
    if (parsedMsg.params.size() < 1) {
        reply = makeReply(serverName, 461, targetNick, "Not enough parameters", parsedMsg.command);
        client.getSendQueue() += reply;
        return;
    }

    std::string channelName = parsedMsg.params[0];
    std::string lowerChannelName = toLower(channelName);

    // channel name s7i7?
    if (lowerChannelName.empty() || (lowerChannelName[0] != '#' && lowerChannelName[0] != '&')) {
        reply = makeReply(serverName, 403, targetNick, "No such channel", channelName);
        client.getSendQueue() += reply;
        return;
    }

    // channel exists?
    Channel* channel = server.getChannel(lowerChannelName);
    if (!channel) {
        reply = makeReply(serverName, 403, targetNick, "No such channel", channelName);
        client.getSendQueue() += reply;
        return;
    }

    // client is in the channel?
    if (!channel->isMember(client.getFd())) {
        reply = makeReply(serverName, 442, targetNick, "You're not on that channel", channelName);
        client.getSendQueue() += reply;
        return;
    }

    // client asking to show the channel topic
    if (parsedMsg.params.size() < 2) {
        if (channel->getTopic().empty()) {
            // numeric reply 331 RPL_NOTOPIC: no topic is set
            reply = makeReply(serverName, 331, targetNick, "No topic is set", channelName);
        }
        else {
            // numeric reply 332 RPL_TOPIC: show the set topic
            reply = makeReply(serverName, 332, targetNick, channel->getTopic(), channelName);
            client.getSendQueue() += reply;
            // numeric reply 333 RPL_TOPICWHOTIME: show who set topic, and when they did
            reply = makeReply(serverName, 333, targetNick, channel->getTopicUpdateTime(), channelName + " " + channel->getTopicUpdateUser()); // it's in the following format: :silver.libera.chat 333 hwa #linux nkukard 1722815284
        }
        client.getSendQueue() += reply;
        return;

    }
    // client wants to change the channel topic
    else {
        std::string newTopic = parsedMsg.params[1];

        // if channel is topic restricted, and client is no operator, can't do shit
        if (channel->isTopicRestricted() && !channel->isOperator(client.getFd())) {
            // numeric reply 482 ERR_CHANOPRIVSNEEDED: need operator privilege
            reply = makeReply(serverName, 482, targetNick, "You're not a channel operator", channelName);
            client.getSendQueue() += reply;
            return ;
        }

        // same topic, no need for changes nor notifying others
        else if (newTopic == channel->getTopic()) {
            return ;
        }

        // change the channel's topic, and notify everyone
        else {
            // build TOPIC message
            std::string topicMsg = ":" + client.getNick() + "!" + client.getUser() + "@localhost TOPIC " + channelName + " :" + newTopic + "\r\n";

            // change topic name for the channel
            channel->setTopic(newTopic);
            channel->setTopicUpdateTime(time(NULL));
            std::string topicChanger = client.getNick() + "!" + client.getUser() + "@" + "localhost";
            channel->setTopicUpdateUser(topicChanger);

            // send TOPIC message to all members in the channel
            const std::set<int>& members = channel->getMembers();
            for (std::set<int>::const_iterator it = members.begin(); it != members.end(); ++it) {
                std::map<int, Client>::iterator iter = server.getMap().find(*it);
                if (iter != server.getMap().end()) {
                    iter->second.getSendQueue() += topicMsg;

                    struct epoll_event current_ev;
                    memset(&current_ev, 0, sizeof(current_ev));
                    current_ev.events = EPOLLOUT | EPOLLIN;
                    current_ev.data.fd = *it;
                    epoll_ctl(server.get_epfd(), EPOLL_CTL_MOD, *it, &current_ev);
                }
            }
        }
    }    
}


//Syntax: INVITE nickname #channel
void handleINVITE(Server& server, Client& client, Command& parsedMsg) {
    std::string targetNick = client.getNick().empty() ? "*" : client.getNick();
    std::string serverName = server.getServerName();
    std::string reply;
    
    if (!client.isRegistered()) {
        reply = makeReply(serverName, 451, targetNick, "Connection not registered");
        client.getSendQueue() += reply;
        return;
    }
    
    if (parsedMsg.params.size() < 2) {
        reply = makeReply(serverName, 461, targetNick, "Not enough parameters", parsedMsg.command);
        client.getSendQueue() += reply;
        return;
    }
    
    std::string targetToInvite = parsedMsg.params[0];
    std::string channelName = parsedMsg.params[1];
    std::string lowerChannelName = toLower(channelName);
    
    if (lowerChannelName.empty() || (lowerChannelName[0] != '#' && lowerChannelName[0] != '&')) {
        reply = makeReply(serverName, 403, targetNick, "No such channel", channelName);
        client.getSendQueue() += reply;
        return;
    }
    
    Channel* channel = server.getChannel(lowerChannelName);
    if (!channel) {
        reply = makeReply(serverName, 403, targetNick, "No such channel", channelName);
        client.getSendQueue() += reply;
        return;
    }
    
    if (channel->isInviteOnly() && !channel->isMember(client.getFd())) {
        reply = makeReply(serverName, 442, targetNick, "You're not on that channel", channelName);
        client.getSendQueue() += reply;
        return;
    }
    
    if (channel->isInviteOnly() && !channel->isOperator(client.getFd())) {
        reply = makeReply(serverName, 482, targetNick, "You're not channel operator", channelName);
        client.getSendQueue() += reply;
        return;
    }
    
    // the target client to invite
    int targetFd = server.nicknameOwner(targetToInvite);
    if (targetFd == -1) {
        reply = makeReply(serverName, 401, targetNick, "No such nick/channel", targetToInvite);
        client.getSendQueue() += reply;
        return;
    }
    
    // is he already in the channel?
    if (channel->isMember(targetFd)) {
        reply = makeReply(serverName, 443, targetNick, "is already on channel", targetToInvite + " " + channelName);
        client.getSendQueue() += reply;
        return;
    }
    
    // nzidouh l invite list
    channel->invite(targetFd);
    
    // send confirmation to inviter
    reply = makeReply(serverName, 341, targetNick, "Inviting " + targetToInvite + " to " + channelName);
    client.getSendQueue() += reply;
    
    // send INVITE notification to target
    std::map<int, Client>::iterator targetIter = server.getMap().find(targetFd);
    if (targetIter != server.getMap().end()) {
        std::string inviteMsg = ":" + client.getNick() + "!" + client.getUser() + "@localhost INVITE " + targetToInvite + " :" + channelName + "\r\n";
        targetIter->second.getSendQueue() += inviteMsg;
        
        struct epoll_event current_ev;
        memset(&current_ev, 0, sizeof(current_ev));
        current_ev.events = EPOLLOUT | EPOLLIN;
        current_ev.data.fd = targetFd;
        epoll_ctl(server.get_epfd(), EPOLL_CTL_MOD, targetFd, &current_ev);
    }
}

void handleMODE(Server& server, Client& client, Command& parsedMsg) {
    std::string targetNick = client.getNick().empty() ? "*" : client.getNick();
    std::string serverName = server.getServerName();
    std::string reply;
    
    if (!client.isRegistered()) {
        reply = makeReply(serverName, 451, targetNick, "Connection not registered");
        client.getSendQueue() += reply;
        return;
    }
    
    if (parsedMsg.params.empty()) {
        reply = makeReply(serverName, 461, targetNick, "Not enough parameters", parsedMsg.command);
        client.getSendQueue() += reply;
        return;
    }
    
    std::string target = parsedMsg.params[0];
    
    // Check if target is a channel (starts with # or &)
    if (!target.empty() && (target[0] == '#' || target[0] == '&')) {
        // CHANNEL MODE
        std::string lowerChannelName = toLower(target);
        
        // Check if channel exists
        Channel* channel = server.getChannel(lowerChannelName);
        if (!channel) {
            reply = makeReply(serverName, 403, targetNick, "No such channel", target);
            client.getSendQueue() += reply;
            return;
        }
        
        // Check if client is in the channel
        if (!channel->isMember(client.getFd())) {
            reply = makeReply(serverName, 442, targetNick, "You're not on that channel", target);
            client.getSendQueue() += reply;
            return;
        }
        
        // Check if client is operator
        if (!channel->isOperator(client.getFd())) {
            reply = makeReply(serverName, 482, targetNick, "You're not channel operator", target);
            client.getSendQueue() += reply;
            return;
        }
        
        // If no mode parameters, show current modes
        if (parsedMsg.params.size() < 2) {
            // Send current modes
            std::string modeString = "+";
            if (channel->isInviteOnly()) modeString += "i";
            if (channel->isTopicRestricted()) modeString += "t";
            if (!channel->getKey().empty()) modeString += "k";
            if (channel->isFull()) modeString += "l";
            
            reply = makeReply(serverName, 324, targetNick, channel->getName() + " " + modeString);
            client.getSendQueue() += reply;
            return;
        }
        
        std::string modeChanges = parsedMsg.params[1];
        bool adding = true;
        
        // Process each mode character
        for (size_t i = 0; i < modeChanges.length(); i++) {
            char c = modeChanges[i];
            
            if (c == '+') {
                adding = true;
            } else if (c == '-') {
                adding = false;
            } else if (c == 'i') {
                // i mode
                channel->setInviteOnly(adding);
            } else if (c == 'o') {
                // o mode
                if (parsedMsg.params.size() < 3) {
                    reply = makeReply(serverName, 461, targetNick, "Not enough parameters for mode +o", parsedMsg.command);
                    client.getSendQueue() += reply;
                    continue;
                }
                
                std::string nickParam = parsedMsg.params[2];
                int targetFd = server.nicknameOwner(nickParam);
                
                if (targetFd == -1) {
                    reply = makeReply(serverName, 401, targetNick, "No such nick", nickParam);
                    client.getSendQueue() += reply;
                    continue;
                }
                
                if (!channel->isMember(targetFd)) {
                    reply = makeReply(serverName, 441, targetNick, "They aren't on that channel", nickParam + " " + channel->getName());
                    client.getSendQueue() += reply;
                    continue;
                }
                
                if (adding) {
                    channel->addOperator(targetFd);
                } else {
                    channel->removeOperator(targetFd);
                }
                
                // Broadcast o mode change to channel
                std::string modeMsg = ":" + client.getNick() + "!" + client.getUser() + "@localhost MODE " + channel->getName() + " " + (adding ? "+" : "-") + "o " + nickParam + "\r\n";
                const std::set<int>& members = channel->getMembers();
                for (std::set<int>::const_iterator it = members.begin(); it != members.end(); ++it) {
                    std::map<int, Client>::iterator iter = server.getMap().find(*it);
                    if (iter != server.getMap().end()) {
                        iter->second.getSendQueue() += modeMsg;
                        
                        struct epoll_event current_ev;
                        memset(&current_ev, 0, sizeof(current_ev));
                        current_ev.events = EPOLLOUT | EPOLLIN;
                        current_ev.data.fd = *it;
                        epoll_ctl(server.get_epfd(), EPOLL_CTL_MOD, *it, &current_ev);
                    }
                }
                
            } else if (c == 'k') {
                // k mode
                if (adding) {
                    if (parsedMsg.params.size() < 3) {
                        reply = makeReply(serverName, 461, targetNick, "Not enough parameters for mode +k", parsedMsg.command);
                        client.getSendQueue() += reply;
                        continue;
                    }
                    channel->setKey(parsedMsg.params[2]);
                } else {
                    channel->setKey("");
                }
            } else if (c == 'l') {
                // l mode
                if (adding) {
                    if (parsedMsg.params.size() < 3) {
                        reply = makeReply(serverName, 461, targetNick, "Not enough parameters for mode +l", parsedMsg.command);
                        client.getSendQueue() += reply;
                        continue;
                    }
                    int limit = atoi(parsedMsg.params[2].c_str());
                    if (limit > 0) {
                        channel->setUserLimit(limit);
                    }
                } else {
                    channel->setUserLimit(0);
                }
            } else if (c == 't') {
                // t mode
                channel->setTopicRestricted(adding);
            }
        }
    } else {
        // we're not required to handle USER MODE
        reply = makeReply(serverName, 502, targetNick, "Cannot change user mode");
        client.getSendQueue() += reply;
    }
}