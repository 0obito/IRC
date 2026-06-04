#include "../includes/Dispatcher.hpp"
#include "../includes/DispatcherUtils.hpp"
#include "../includes/Server.hpp"
#include "../includes/Utils.hpp"
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
    // std::cout << finalMsg;
    return ;
}

void registerClient(Client& client, const std::string serverName) {
    if (!client.isRegistered()) {
        if (client.getPassOk() && client.getNickOk() && client.getUserOk()) {
            // turn on registration flag
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

void handleQUIT(Server& server, Client& client, Command& parsedMsg) {
    (void)server;
    (void)client;
    (void)parsedMsg;
    // later machi db, shouldn't take any time;
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
    if (parsedMsg.params[0] == server.get_password()) {
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
    if (server.isNicknameTaken(nickName) != -1) {
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
        int targetFD = server.isNicknameTaken(singleTarget);
        if (targetFD == -1) {
            reply = makeReply(serverName, 401, senderNick, "No such nick/channel", singleTarget);
            client.getSendQueue() += reply;
            continue ;
        }
        std::map<int, Client>::iterator iter = server.mapGetter().find(targetFD);
        if (iter != server.mapGetter().end()) {
            reply = ":" + client.getNick() + "!" + client.getUser() + "@localhost " 
                    + parsedMsg.command + " " + iter->second.getNick() + " :" + messageText + "\r\n";
            iter->second.getSendQueue() += reply;
            struct epoll_event current_ev;
            memset(&current_ev, 0, sizeof(current_ev));
            current_ev.events = EPOLLOUT | EPOLLIN;
            current_ev.data.fd = iter->first;
            epoll_ctl(server.get_epfd(), EPOLL_CTL_MOD, iter->first, &current_ev);
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
        int targetFD = server.isNicknameTaken(singleTarget);
        if (targetFD == -1) {
            continue;
        }
        std::map<int, Client>::iterator iter = server.mapGetter().find(targetFD);
        if (iter != server.mapGetter().end()) {
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
