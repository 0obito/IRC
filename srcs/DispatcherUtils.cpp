#include "includes/Dispatcher.hpp"
#include "includes/DispatcherUtils.hpp"

void handleCAP(Server& server, Client& client, Command& parsedMsg) {
    std::cout << "CAP * LS :" << std::endl;
}

void handlePASS(Server& server, Client& client, Command& parsedMsg) {
    if (client.isRegistered == true) {
        std::cout << "ERR_ALREADYREGISTERED (462)" << std::endl;
        return ;
    }
    if (parsedMsg.params.empty() == true) {
        std::cout << "ERR_NEEDMOREPARAMS (461)" << std::endl;
        return ;
    }
    if (parsedMsg.params[0] == server.getPassword()) {
        client.setPassOk(true);
        return ;
    }
    else {
        std::cout << "ERR_PASSWDMISMATCH (464)" << std::endl;
        return ;
    }
}

bool nickIsValid(std::string &nickName) {
    if (nickname.empty())
        return false;
    if (nickname.find(' ') != std::string::npos || nickname.find(',') != std::string::npos || nickname.find('*') != std::string::npos || nickname.find('.') != std::string::npos
        || nickname.find('?') != std::string::npos || nickname.find('!') != std::string::npos || nickname.find('@') != std::string::npos)
        return false;
    if (nickname[0] == '$' || nickname[0] == ':' || nickname[0] == '#' || nickname[0] == '&'
        || nickname[0] == '~' || nickname[0] == '%' || nickname[0] == '+')
        return false;
    return true;
}

void handleNICK(Server& server, Client& client, Command& parsedMsg) {
    if (parsedMsg.params.empty() == true) {
        // NR 431: Returned when a nickname parameter is expected for a command but isn’t given.
        std::cout << "ERR_NONICKNAMEGIVEN (431)" << std::endl;
        return ;
    }
    std::string nickName = parsedMsg.params[0];
    if (client.getNickOk == true && client.getNick == nickName) {
        // the requested nickname is the same as the existing older nickname, nothing to do.
        return ;
    }
    if (server.isNicknameTaken(parsedMsg.params[0]) == true) {
        // NR 433: Returned when a NICK command cannot be successfully completed as the desired nickname is already in use on the network.
        std::cout << "ERR_NICKNAMEINUSE (433)" << std::endl;
        return ;
    }
    if (nickIsValid(nickName) == true) {
        // the requested nickname is fine, set it.
        client.setNick(nickName);
        return ;
    }
    else {
        // NR 432: Returned when a NICK command cannot be successfully completed as the desired nickname contains characters that are disallowed by the server.
        std::cout << "ERR_ERRONEUSNICKNAME (432)" << std::endl;
        return ;
    }
}

void handleUSER(Server& server, Client& client, Command& parsedMsg) {
    ;
}
