#include "includes/Dispatcher.hpp"
#include "includes/DispatcherUtils.hpp"
#include "includes/Server.hpp"

void handleCAP(Server& server, Client& client, Command& parsedMsg) {
    std::cout << "CAP * LS :" << std::endl;
}

void handlePASS(Server& server, Client& client, Command& parsedMsg) {
    if (client.isRegistered()) {
        std::cout << "ERR_ALREADYREGISTERED (462)" << std::endl;
        return ;
    }
    if (parsedMsg.params.empty()) {
        std::cout << "ERR_NEEDMOREPARAMS (461)" << std::endl;
        return ;
    }
    if (parsedMsg.params[0] == server.get_password()) {
        client.setPassOk(true);
        return ;
    }
    else {
        std::cout << "ERR_PASSWDMISMATCH (464)" << std::endl;
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
    if (!client.getPassOk()) {
        // needs a more conveninet message than this i believe.
        std::cout << "ERR_PASSWDMISMATCH (464) | Password was not supplied" << std::endl;
        return ;
    }
    if (parsedMsg.params.empty()) {
        // NR 431: Returned when a nickname parameter is expected for a command but isn’t given.
        std::cout << "ERR_NONICKNAMEGIVEN (431)" << std::endl;
        return ;
    }
    std::string nickName = parsedMsg.params[0];
    if (client.getNickOk() && client.getNick() == nickName) {
        // [CHANGE] the requested nickname is the same as the existing older nickname, nothing to do.
        return ;
    }
    if (server.isNicknameTaken(parsedMsg.params[0])) {
        // NR 433: Returned when a NICK command cannot be successfully completed as the desired nickname is already in use on the network.
        std::cout << "ERR_NICKNAMEINUSE (433)" << std::endl;
        return ;
    }
    if (nickIsValid(nickName)) {
        // the requested nickname is fine, set it.
        client.setNick(nickName);
        registerClient(server, client, parsedMsg);
        return ;
    }
    else {
        // NR 432: Returned when a NICK command cannot be successfully completed as the desired nickname contains characters that are disallowed by the server.
        std::cout << "ERR_ERRONEUSNICKNAME (432)" << std::endl;
        return ;
    }
}

void handleUSER(Server& server, Client& client, Command& parsedMsg) {
    if (client.isRegistered()) {
        std::cout << "ERR_ALREADYREGISTERED (462)" << std::endl;
        return ;
    }
    if (!client.getPassOk()) {
        std::cout << "ERR_PASSWDMISMATCH (464) | Password was not supplied" << std::endl;
        return ;
    }
    if (parsedMsg.params.empty() || parsedMsg.params.size() < 4) {
        std::cout << "ERR_NEEDMOREPARAMS (461)" << std::endl;
        return ;
    }
    client.setUser(parsedMsg.params[0]);
    client.setRealname(parsedMsg.params[3]);
    registerClient(client);
}

void registerClient(Client& client) {
    // to be triggered in both NICK and USER, checks if the client should be registered now or not.
    if (!client.isRegistered()) {
        if (client.getPassOk() && client.getNickOk() && client.getUserOk()) {
            client.setRegistered(true);
            std::cout << "" << std::endl;
        }
    }
    return ;
}
