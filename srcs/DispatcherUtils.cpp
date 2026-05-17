#include "../includes/Dispatcher.hpp"
#include "../includes/DispatcherUtils.hpp"
#include "../includes/Server.hpp"
#include "../includes/Utils.hpp"

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
    std::string nickName = parsedMsg.params[0];
    if (client.getNickOk() && client.getNick() == nickName) {
        return ;
    }
    if (server.isNicknameTaken(nickName)) {
        reply = makeReply(serverName, 433, targetNick, "Nickname is already in use", nickName);
        client.getSendQueue() += reply;
        // std::cout << "ERR_NICKNAMEINUSE (433)" << std::endl;
        return ;
    }
    if (!nickIsValid(nickName)) {
        reply = makeReply(serverName, 433, targetNick, "Erroneous nickname", nickName);
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
    std::string targetNick = client.getNick();
    std::string serverName = server.getServerName();
    std::string reply;

    if (parsedMsg.params.empty()) {
        reply = makeReply(serverName, 412, targetNick, "No text to send");
        client.getSendQueue() += reply;
        // std::cout << "ERR_NOTEXTTOSEND (412)" << std::endl;
        return ;
    }
    size_t params_num = parsedMsg.params.size();
    if (params_num < 2) {
        reply = makeReply(serverName, 999, targetNick, "No nick to send to");
        client.getSendQueue() += reply;
        // std::cout << "no nick to send to (999)" << std::endl;
        return ;
    }
    reply = makeReply(serverName, 464, targetNick, "Password incorrect");
    client.getSendQueue() += reply;
    // std::cout << "ERR_PASSWDMISMATCH (464)" << std::endl;
    return ;
}
