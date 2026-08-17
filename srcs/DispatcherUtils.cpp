#include "../includes/DispatcherUtils.hpp"

#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Parser.hpp"
#include "../includes/Utils.hpp"
#include "../includes/Channel.hpp"

//////////////////////////////////////////////////////////////////////////////// DONE
void welcomingSeq(Client& client, const std::string serverName) {
    std::string nick = client.getNick();
    std::string user = client.getUser();
    std::string host = client.getHostname();
    std::string version = " 1.0";
    std::string userModes = " o";
    std::string chanModes = " itkol";
    std::stringstream ss;

    ss << ":" << serverName << " 001 " << nick;
    ss << " :Welcome To ircDyalna Internet Relay Chat Network " << nick << "!" << user << "@" << host <<"\r\n";

    ss << ":" << serverName << " 002 " << nick;
    ss << " :Your host is " << serverName << ", running version" << version << "\r\n";

    ss << ":" << serverName << " 003 " << nick;
    ss << " :This server was created not so long ago\r\n";

    ss << ":" << serverName << " 004 " << nick;
    ss << " " << serverName << version << userModes << chanModes << "\r\n";

    ss << ":" << serverName << " 005 " << nick;
    ss << " CHANTYPES=#& PREFIX=(o)@ CHANNELLEN=50 NICKLEN=30 NETWORK=NetworkDyalna :are supported by this server\r\n";

    ss << ":" << serverName << " 422 " << nick;
    ss << " :MOTD File is missing\r\n";

    std::string finalMsg = ss.str();
    client.getSendQueue() += finalMsg;
    return ;
}

//////////////////////////////////////////////////////////////////////////////// DONE
void registerClient(Client& client, const std::string serverName) {
    if (client.getPassOk() && client.getNickOk() && client.getUserOk()) {
        client.setRegistered(true);
        welcomingSeq(client, serverName);
    }
    return ;
}

//////////////////////////////////////////////////////////////////////////////// DONE
void handlePASS(Server& server, Client& client, Command& parsedMsg) {
    std::string senderNick = client.getNick().empty() ? "*" : client.getNick();
    std::string serverName = server.getServerName();
    std::string reply;

    if (client.isRegistered()) {
        reply = makeReply(serverName, 462, senderNick, "Unauthorized command (already registered)");
        client.getSendQueue() += reply;
        return ;
    }
    if (parsedMsg.params.empty()) {
        reply = makeReply(serverName, 461, senderNick, "Not enough parameters", parsedMsg.command);
        client.getSendQueue() += reply;
        return ;
    }
    if (parsedMsg.params[0] == server.getPassword()) {
        client.setPassOk(true);
        return ;
    }
    else {
        client.setPassOk(false);
        reply = makeReply(serverName, 464, senderNick, "Password incorrect");
        client.getSendQueue() += reply;
        return ;
    }
}

//////////////////////////////////////////////////////////////////////////////// DONE
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

//////////////////////////////////////////////////////////////////////////////// DONE
void handleNICK(Server& server, Client& client, Command& parsedMsg) {
    std::string senderNick = client.getNick().empty() ? "*" : client.getNick();
    std::string serverName = server.getServerName();
    std::string reply;

    // password first my friend
    if (!client.getPassOk()) {
        reply = makeReply(serverName, 464, senderNick, "Password incorrect");
        client.getSendQueue() += reply;
        return ;
    }

    // no nickname was passed
    if (parsedMsg.params.empty()) {
        reply = makeReply(serverName, 431, senderNick, "No nickname given");
        client.getSendQueue() += reply;
        return ;
    }

    std::string nickName = parsedMsg.params[0];

    // nickname is invalid
    if (!nickIsValid(nickName)) {
        reply = makeReply(serverName, 432, senderNick, "Erroneous nickname", nickName);
        client.getSendQueue() += reply;
        return ;
    }

    int nickOwner = server.nicknameOwner(nickName);

    // nickname is used by someone else
    if (nickOwner != -1 && nickOwner != client.getFd()) {
        reply = makeReply(serverName, 433, senderNick, "Nickname is already in use", nickName);
        client.getSendQueue() += reply;
        return ;
    }

    client.setNick(nickName);
    client.setNickOk(true);

    if (client.isRegistered()) {
        // build NICK message
        std::string nickMsg = ":" + senderNick + "!" + client.getUser() + "@" + client.getHostname() + " NICK :" + nickName + "\r\n";

        // send to the sender first
        client.getSendQueue() += nickMsg;

        // Track clients notified
        std::set<int> notifiedFds;
        notifiedFds.insert(client.getFd());

        // Loop through all channels joined by client
        const std::set<std::string>& myChannels = client.getJoinedChannels();
        for (std::set<std::string>::const_iterator chIt = myChannels.begin(); chIt != myChannels.end(); ++chIt) {
            Channel* channel = server.getChannel(*chIt);
            if (channel) {
                // loop through ga3 members of channel
                const std::set<int>& members = channel->getMembers();
                for (std::set<int>::const_iterator memIt = members.begin(); memIt != members.end(); ++memIt) {
                    // notify each client once
                    if (notifiedFds.find(*memIt) == notifiedFds.end()) {
                        server.queueResponse(*memIt, nickMsg);
                        notifiedFds.insert(*memIt);
                    }
                }
            }
        }
    }
    else {
        registerClient(client, serverName);
    }
    return ;
}

//////////////////////////////////////////////////////////////////////////////// DONE
void handleUSER(Server& server, Client& client, Command& parsedMsg) {
    std::string senderNick = client.getNick().empty() ? "*" : client.getNick();
    std::string serverName = server.getServerName();
    std::string reply;

    // are u already registered?
    if (client.isRegistered()) {
        reply = makeReply(serverName, 462, senderNick, "Unauthorized command (already registered)");
        client.getSendQueue() += reply;
        return ;
    }

    // where's your password dude
    if (!client.getPassOk()) {
        reply = makeReply(serverName, 464, senderNick, "Password incorrect");
        client.getSendQueue() += reply;
        return ;
    }

    // give me correct params my dude
    if (parsedMsg.params.empty() || parsedMsg.params.size() < 4 || parsedMsg.params[3].empty()) {
        reply = makeReply(serverName, 461, senderNick, "Not enough parameters", parsedMsg.command);
        client.getSendQueue() += reply;
        return ;
    }
    client.setUser(parsedMsg.params[0]);
    client.setRealname(parsedMsg.params[3]);
    client.setUserOk(true);
    registerClient(client, serverName);
}

void handlePRIVMSG(Server& server, Client& client, Command& parsedMsg) {
    std::string senderNick = client.getNick();
    std::string serverName = server.getServerName();
    std::string reply;

    if (!client.isRegistered()) {
        reply = makeReply(serverName, 451, senderNick, "Connection not registered");
        client.getSendQueue() += reply;
        return ;
    }

    if (parsedMsg.params.empty()) {
        reply = makeReply(serverName, 411, senderNick, "No recipient given (PRIVMSG)");
        client.getSendQueue() += reply;
        return ;
    }
    if (parsedMsg.params.size() < 2) {
        reply = makeReply(serverName, 412, senderNick, "No text to send");
        client.getSendQueue() += reply;
        return ;
    }
    if (parsedMsg.params.size() > 2) {
        reply = makeReply(serverName, 461, senderNick, "Syntax error", parsedMsg.command);
        client.getSendQueue() += reply;
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

void handlePING(Server& server, Client& client, Command& parsedMsg) {
    std::string senderNick = client.getNick().empty() ? "*" : client.getNick();
    std::string serverName = server.getServerName();
    std::string reply;

    if (!client.isRegistered()) {
        reply = makeReply(serverName, 451, senderNick, "Connection not registered");
        client.getSendQueue() += reply;
        return ;
    }

    if (parsedMsg.params.empty()) {
        reply = makeReply(serverName, 409, senderNick, "No origin specified");
        client.getSendQueue() += reply;
        return ;
    }
    if (parsedMsg.params.size() > 1 && parsedMsg.params[1] != serverName) {
        reply = makeReply(serverName, 402, senderNick, "No such server", parsedMsg.params[1]);
        client.getSendQueue() += reply;
        return ;
    }

    std::stringstream ss;
    ss << ":" << serverName << " PONG " << serverName << " :" << parsedMsg.params[0] << "\r\n";
    reply = ss.str();
    client.getSendQueue() += reply;
    return ;
}

void handlePONG(Server& server, Client& client, Command& parsedMsg) {
    std::string senderNick = client.getNick();
    std::string serverName = server.getServerName();
    std::string reply;

    if (!client.isRegistered()) {
        return ;
    }

    if (parsedMsg.params.empty()) {
        reply = makeReply(serverName, 409, senderNick, "No origin specified");
        client.getSendQueue() += reply;
        return ;
    }
    if (parsedMsg.params.size() > 2) {
        reply = makeReply(serverName, 461, senderNick, "Syntax error", parsedMsg.command);
        client.getSendQueue() += reply;
        return ;
    }
}

void handleJOIN(Server& server, Client& client, Command& parsedMsg) {
    std::string senderNick = client.getNick();
    std::string serverName = server.getServerName();
    std::string reply;

    // Check if client is registered
    if (!client.isRegistered()) {
        reply = makeReply(serverName, 451, senderNick, "Connection not registered");
        client.getSendQueue() += reply;
        return;
    }

    // Check if enough parameters
    if (parsedMsg.params.empty()) {
        reply = makeReply(serverName, 461, senderNick, "Not enough parameters", parsedMsg.command);
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
            reply = makeReply(serverName, 476, senderNick, "Invalid channel name", channelName);
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
            reply = makeReply(serverName, 473, senderNick, "Cannot join channel (+i)", channelName);
            client.getSendQueue() += reply;
            continue;
        }

        // Check key (if channel has a key and it doesn't match)
        if (!channel->getKey().empty() && channel->getKey() != channelKey) {
            reply = makeReply(serverName, 475, senderNick, "Cannot join channel (+k)", channelName);
            client.getSendQueue() += reply;
            continue;
        }

        // Check user limit
        if (channel->isFull()) {
            reply = makeReply(serverName, 471, senderNick, "Cannot join channel (+l)", channelName);
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
        reply = ":" + client.getNick() + "!" + client.getUser() + "@" + client.getHostname() + " JOIN " + channelName + "\r\n";
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
                    epoll_ctl(server.getEPFD(), EPOLL_CTL_MOD, *it, &current_ev);
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

// DONE
void handlePART(Server& server, Client& client, Command& parsedMsg) {
    std::string senderNick = client.getNick();
    std::string serverName = server.getServerName();
    std::string reply;

    // Check if client is registered
    if (!client.isRegistered()) {
        reply = makeReply(serverName, 451, senderNick, "Connection not registered");
        client.getSendQueue() += reply;
        return;
    }

    // Check if enough parameters
    if (parsedMsg.params.empty()) {
        reply = makeReply(serverName, 461, senderNick, "Not enough parameters", parsedMsg.command);
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
            reply = makeReply(serverName, 403, senderNick, "No such channel", channelName);
            client.getSendQueue() += reply;
            continue;
        }

        // Check if channel exists
        Channel* channel = server.getChannel(lowerChannelName);
        if (!channel) {
            reply = makeReply(serverName, 403, senderNick, "No such channel", channelName);
            client.getSendQueue() += reply;
            continue;
        }

        // Check if client is in the channel
        if (!channel->isMember(client.getFd())) {
            reply = makeReply(serverName, 442, senderNick, "You're not on that channel", channelName);
            client.getSendQueue() += reply;
            continue;
        }

        // Build PART message (broadcast to ALL members including the departing user)
        std::string partMsg;
        if (reason.empty()) {
            partMsg = ":" + client.getNick() + "!" + client.getUser() + "@" + client.getHostname() + " PART " + channelName + "\r\n";
        } else {
            partMsg = ":" + client.getNick() + "!" + client.getUser() + "@" + client.getHostname() + " PART " + channelName + " :" + reason + "\r\n";
        }

        // Send PART to all members in the channel (including the departing user)
        const std::set<int>& members = channel->getMembers();
        for (std::set<int>::const_iterator it = members.begin(); it != members.end(); ++it) {
            server.queueResponse(*it, partMsg);
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
    std::string senderNick = client.getNick().empty() ? "*" : client.getNick();
    std::string serverName = server.getServerName();
    std::string reply;
    
    // registered?
    if (!client.isRegistered()) {
        reply = makeReply(serverName, 451, senderNick, "Connection not registered");
        client.getSendQueue() += reply;
        return;
    }
    
    // enough parameters?
    if (parsedMsg.params.size() < 2) {
        reply = makeReply(serverName, 461, senderNick, "Not enough parameters", parsedMsg.command);
        client.getSendQueue() += reply;
        return;
    }
    
    std::string channelName = parsedMsg.params[0];
    std::string targetToKick = parsedMsg.params[1];
    std::string reason = parsedMsg.params.size() > 2 ? parsedMsg.params[2] : "No reason given";
    std::string lowerChannelName = toLower(channelName);
    
    // channel name s7i7?
    if (lowerChannelName.empty() || (lowerChannelName[0] != '#' && lowerChannelName[0] != '&')) {
        reply = makeReply(serverName, 403, senderNick, "No such channel", channelName);
        client.getSendQueue() += reply;
        return;
    }
    
    // channel exists?
    Channel* channel = server.getChannel(lowerChannelName);
    if (!channel) {
        reply = makeReply(serverName, 403, senderNick, "No such channel", channelName);
        client.getSendQueue() += reply;
        return;
    }
    
    // client is in the channel?
    if (!channel->isMember(client.getFd())) {
        reply = makeReply(serverName, 442, senderNick, "You're not on that channel", channelName);
        client.getSendQueue() += reply;
        return;
    }
    
    // client is a channel operator?
    if (!channel->isOperator(client.getFd())) {
        reply = makeReply(serverName, 482, senderNick, "You're not channel operator", channelName);
        client.getSendQueue() += reply;
        return;
    }
    
    // target client nickname kayn?
    int targetFd = server.nicknameOwner(targetToKick);
    if (targetFd == -1) {
        reply = makeReply(serverName, 401, senderNick, "No such nick/channel", targetToKick);
        client.getSendQueue() += reply;
        return;
    }
    
    // target is in the channel?
    if (!channel->isMember(targetFd)) {
        reply = makeReply(serverName, 441, senderNick, "They aren't on that channel", targetToKick + " " + channelName);
        client.getSendQueue() += reply;
        return;
    }
    
    // kick yourself? la asa7bi!
    if (targetFd == client.getFd()) {
        reply = makeReply(serverName, 482, senderNick, "You can't kick yourself", channelName);
        client.getSendQueue() += reply;
        return;
    }
    
    // build KICK message
    std::string kickMsg = ":" + client.getNick() + "!" + client.getUser() + "@" + client.getHostname() + " KICK " + channelName + " " + targetToKick + " :" + reason + "\r\n";
    
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
            epoll_ctl(server.getEPFD(), EPOLL_CTL_MOD, *it, &current_ev);
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
    std::string senderNick = client.getNick().empty() ? "*" : client.getNick();
    std::string serverName = server.getServerName();
    std::string reply;
    
    // registered?
    if (!client.isRegistered()) {
        reply = makeReply(serverName, 451, senderNick, "Connection not registered");
        client.getSendQueue() += reply;
        return;
    }

    // enough parameters?
    if (parsedMsg.params.size() < 1) {
        reply = makeReply(serverName, 461, senderNick, "Not enough parameters", parsedMsg.command);
        client.getSendQueue() += reply;
        return;
    }

    std::string channelName = parsedMsg.params[0];
    std::string lowerChannelName = toLower(channelName);

    // channel name s7i7?
    if (lowerChannelName.empty() || (lowerChannelName[0] != '#' && lowerChannelName[0] != '&')) {
        reply = makeReply(serverName, 403, senderNick, "No such channel", channelName);
        client.getSendQueue() += reply;
        return;
    }

    // channel exists?
    Channel* channel = server.getChannel(lowerChannelName);
    if (!channel) {
        reply = makeReply(serverName, 403, senderNick, "No such channel", channelName);
        client.getSendQueue() += reply;
        return;
    }

    // client is in the channel?
    if (!channel->isMember(client.getFd())) {
        reply = makeReply(serverName, 442, senderNick, "You're not on that channel", channelName);
        client.getSendQueue() += reply;
        return;
    }

    // client asking to show the channel topic
    if (parsedMsg.params.size() < 2) {
        if (channel->getTopic().empty()) {
            // numeric reply 331 RPL_NOTOPIC: no topic is set
            reply = makeReply(serverName, 331, senderNick, "No topic is set", channelName);
        }
        else {
            // numeric reply 332 RPL_TOPIC: show the set topic
            reply = makeReply(serverName, 332, senderNick, channel->getTopic(), channelName);
            client.getSendQueue() += reply;
            // numeric reply 333 RPL_TOPICWHOTIME: show who set topic, and when they did
            reply = makeReply(serverName, 333, senderNick, channel->getTopicUpdateTime(), channelName + " " + channel->getTopicUpdateUser()); // it's in the following format: :silver.libera.chat 333 hwa #linux nkukard 1722815284
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
            reply = makeReply(serverName, 482, senderNick, "You're not a channel operator", channelName);
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
            std::string topicMsg = ":" + client.getNick() + "!" + client.getUser() + "@" + client.getHostname() + " TOPIC " + channelName + " :" + newTopic + "\r\n";

            // change topic name for the channel
            channel->setTopic(newTopic);
            channel->setTopicUpdateTime(time(NULL));
            std::string topicChanger = client.getNick() + "!" + client.getUser() + "@" + client.getHostname();
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
                    epoll_ctl(server.getEPFD(), EPOLL_CTL_MOD, *it, &current_ev);
                }
            }
        }
    }    
}


//Syntax: INVITE nickname #channel
void handleINVITE(Server& server, Client& client, Command& parsedMsg) {
    std::string senderNick = client.getNick().empty() ? "*" : client.getNick();
    std::string serverName = server.getServerName();
    std::string reply;
    
    if (!client.isRegistered()) {
        reply = makeReply(serverName, 451, senderNick, "Connection not registered");
        client.getSendQueue() += reply;
        return;
    }
    
    if (parsedMsg.params.size() < 2) {
        reply = makeReply(serverName, 461, senderNick, "Not enough parameters", parsedMsg.command);
        client.getSendQueue() += reply;
        return;
    }
    
    std::string targetToInvite = parsedMsg.params[0];
    std::string channelName = parsedMsg.params[1];
    std::string lowerChannelName = toLower(channelName);
    
    if (lowerChannelName.empty() || (lowerChannelName[0] != '#' && lowerChannelName[0] != '&')) {
        reply = makeReply(serverName, 403, senderNick, "No such channel", channelName);
        client.getSendQueue() += reply;
        return;
    }
    
    Channel* channel = server.getChannel(lowerChannelName);
    if (!channel) {
        reply = makeReply(serverName, 403, senderNick, "No such channel", channelName);
        client.getSendQueue() += reply;
        return;
    }
    
    if (channel->isInviteOnly() && !channel->isMember(client.getFd())) {
        reply = makeReply(serverName, 442, senderNick, "You're not on that channel", channelName);
        client.getSendQueue() += reply;
        return;
    }
    
    if (channel->isInviteOnly() && !channel->isOperator(client.getFd())) {
        reply = makeReply(serverName, 482, senderNick, "You're not channel operator", channelName);
        client.getSendQueue() += reply;
        return;
    }
    
    // the target client to invite
    int targetFd = server.nicknameOwner(targetToInvite);
    if (targetFd == -1) {
        reply = makeReply(serverName, 401, senderNick, "No such nick/channel", targetToInvite);
        client.getSendQueue() += reply;
        return;
    }
    
    // is he already in the channel?
    if (channel->isMember(targetFd)) {
        reply = makeReply(serverName, 443, senderNick, "is already on channel", targetToInvite + " " + channelName);
        client.getSendQueue() += reply;
        return;
    }
    
    // nzidouh l invite list
    channel->invite(targetFd);
    
    // send confirmation to inviter
    reply = makeReply(serverName, 341, senderNick, "Inviting " + targetToInvite + " to " + channelName);
    client.getSendQueue() += reply;
    
    // send INVITE notification to target
    std::map<int, Client>::iterator targetIter = server.getMap().find(targetFd);
    if (targetIter != server.getMap().end()) {
        std::string inviteMsg = ":" + client.getNick() + "!" + client.getUser() + "@" + client.getHostname() + " INVITE " + targetToInvite + " :" + channelName + "\r\n";
        targetIter->second.getSendQueue() += inviteMsg;

        struct epoll_event current_ev;
        memset(&current_ev, 0, sizeof(current_ev));
        current_ev.events = EPOLLOUT | EPOLLIN;
        current_ev.data.fd = targetFd;
        epoll_ctl(server.getEPFD(), EPOLL_CTL_MOD, targetFd, &current_ev);
    }
}

void handleMODE(Server& server, Client& client, Command& parsedMsg) {
    std::string senderNick = client.getNick().empty() ? "*" : client.getNick();
    std::string serverName = server.getServerName();
    std::string reply;
    
    if (!client.isRegistered()) {
        reply = makeReply(serverName, 451, senderNick, "Connection not registered");
        client.getSendQueue() += reply;
        return;
    }
    
    if (parsedMsg.params.empty()) {
        reply = makeReply(serverName, 461, senderNick, "Not enough parameters", parsedMsg.command);
        client.getSendQueue() += reply;
        return;
    }

    std::string target = parsedMsg.params[0];

    if (!target.empty() && (target[0] == '#' || target[0] == '&')) {
        std::string lowerChannelName = toLower(target);
        
        Channel* channel = server.getChannel(lowerChannelName);
        if (!channel) {
            reply = makeReply(serverName, 403, senderNick, "No such channel", target);
            client.getSendQueue() += reply;
            return;
        }

        if (!channel->isMember(client.getFd())) {
            reply = makeReply(serverName, 442, senderNick, "You're not on that channel", target);
            client.getSendQueue() += reply;
            return;
        }
        
        if (!channel->isOperator(client.getFd())) {
            reply = makeReply(serverName, 482, senderNick, "You're not channel operator", target);
            client.getSendQueue() += reply;
            return;
        }
        
        if (parsedMsg.params.size() < 2) {
            std::string modeString = "+";
            if (channel->isInviteOnly()) modeString += "i";
            if (channel->isTopicRestricted()) modeString += "t";
            if (!channel->getKey().empty()) modeString += "k";
            if (channel->isFull()) modeString += "l";
            
            reply = makeReply(serverName, 324, senderNick, channel->getName() + " " + modeString);
            client.getSendQueue() += reply;
            return;
        }
        
        std::string modeChanges = parsedMsg.params[1];
        bool adding = true;
        bool modeChanged = false;
        std::string appliedChanges = "";  // Track what was actually applied
        std::string appliedParams = "";    // Track parameters used
        size_t paramIndex = 2;
        
        for (size_t i = 0; i < modeChanges.length(); i++) {
            char c = modeChanges[i];
            
            if (c == '+') {
                adding = true;
                appliedChanges += "+";
            } else if (c == '-') {
                adding = false;
                appliedChanges += "-";
            } else if (c == 'i') {
                channel->setInviteOnly(adding);
                appliedChanges += "i";
                modeChanged = true;
                
            } else if (c == 'o') {
                if (paramIndex >= parsedMsg.params.size()) {
                    reply = makeReply(serverName, 461, senderNick, "Not enough parameters for mode o", parsedMsg.command);
                    client.getSendQueue() += reply;
                    continue;
                }
                
                std::string nickParam = parsedMsg.params[paramIndex++];
                int targetFd = server.nicknameOwner(nickParam);
                
                if (targetFd == -1) {
                    reply = makeReply(serverName, 401, senderNick, "No such nick", nickParam);
                    client.getSendQueue() += reply;
                    continue;
                }
                
                if (!channel->isMember(targetFd)) {
                    reply = makeReply(serverName, 441, senderNick, "They aren't on that channel", nickParam + " " + channel->getName());
                    client.getSendQueue() += reply;
                    continue;
                }
                
                if (adding) {
                    channel->addOperator(targetFd);
                } else {
                    channel->removeOperator(targetFd);
                }
                appliedChanges += "o";
                appliedParams += " " + nickParam;
                modeChanged = true;
                
            } else if (c == 'k') {
                if (adding) {
                    if (paramIndex >= parsedMsg.params.size()) {
                        reply = makeReply(serverName, 461, senderNick, "Not enough parameters for mode +k", parsedMsg.command);
                        client.getSendQueue() += reply;
                        continue;
                    }
                    channel->setKey(parsedMsg.params[paramIndex++]);
                    appliedParams += " " + parsedMsg.params[paramIndex - 1];
                } else {
                    channel->setKey("");
                }
                appliedChanges += "k";
                modeChanged = true;
                
            } else if (c == 'l') {
                if (adding) {
                    if (paramIndex >= parsedMsg.params.size()) {
                        reply = makeReply(serverName, 461, senderNick, "Not enough parameters for mode +l", parsedMsg.command);
                        client.getSendQueue() += reply;
                        continue;
                    }
                    int limit = atoi(parsedMsg.params[paramIndex++].c_str());
                    if (limit > 0) {
                        channel->setUserLimit(limit);
                        std::stringstream ss;
                        ss << limit;
                        appliedParams += " " + ss.str();
                    }
                } else {
                    channel->setUserLimit(0);
                }
                appliedChanges += "l";
                modeChanged = true;
                
            } else if (c == 't') {
                channel->setTopicRestricted(adding);
                appliedChanges += "t";
                modeChanged = true;
            }
        }
        
        // BROADCAST TO ALL CHANNEL MEMBERS
        if (modeChanged && !appliedChanges.empty()) {
            std::string modeMsg = ":" + client.getNick() + "!" + client.getUser() + "@" + client.getHostname() + " MODE " + channel->getName() + " " + appliedChanges + appliedParams + "\r\n";
            
            const std::set<int>& members = channel->getMembers();
            for (std::set<int>::const_iterator it = members.begin(); it != members.end(); ++it) {
                std::map<int, Client>::iterator iter = server.getMap().find(*it);
                if (iter != server.getMap().end()) {
                    iter->second.getSendQueue() += modeMsg;
                    
                    struct epoll_event current_ev;
                    memset(&current_ev, 0, sizeof(current_ev));
                    current_ev.events = EPOLLOUT | EPOLLIN;
                    current_ev.data.fd = *it;
                    epoll_ctl(server.getEPFD(), EPOLL_CTL_MOD, *it, &current_ev);
                }
            }
        }
        
    } else {
        reply = makeReply(serverName, 502, senderNick, "Cannot change user mode");
        client.getSendQueue() += reply;
    }
}

// bot handlers

void    handleJOKE(int socketfd, std::string target, Command cmd) {
    (void)cmd;
    const char* jokes[] = {
            "Why do programmers prefer dark mode? Because light attracts bugs.",
            "How many programmers does it take to change a light bulb? None, that's a hardware problem.",
            "I would tell you a UDP joke, but you might not get it.",
            "There are 10 types of people in the world: those who understand binary, and those who don't."
        };
    int num_jokes = sizeof(jokes) / sizeof(jokes[0]);
    int random_index = std::rand() % num_jokes;
    std::string reply = "PRIVMSG " + target + " :" + jokes[random_index] + "\r\n";
    send(socketfd, reply.c_str(), reply.length(), MSG_DONTWAIT);
}

void    handleANONYM(int socketfd, std::string target, Command cmd) {
    std::string reply;

    if (cmd.params.size() == 2)
        reply = "PRIVMSG " + cmd.params[0] + " :" + "ANONYM: " + cmd.params[1] + "\r\n";
    else {
        reply = "PRIVMSG " + target + " :ANONYM [target nick name] [message]" + "\r\n";
    }
    send(socketfd, reply.c_str(), reply.length(), MSG_DONTWAIT);
}