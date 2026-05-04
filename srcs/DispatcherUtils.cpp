#include "includes/Dispatcher.hpp"
#include "includes/DispatcherUtils.hpp"

void handlePASS(Client& client, Server& server, Command& parsedMsg) {
    if (client.isRegistered) {
        std::cout << "ERR_ALREADYREGISTERED (462)" << std::endl;
        return ;
    }
    if (sizeof(parsedMsg.params) == 0) {
        std::cout << "ERR_NEEDMOREPARAMS (461)" << std::endl;
        return ;
    }
    if (parsedMsg.params[0] == server.getPassword()) {
        client.setPassOk(true);
        return ;
    }
    else {
        std::cout << "ERR_NEEDMOREPARAMS (461)" << std::endl;
        return ;
    }
}

void handleNICK(Client& client, Server& server, Command& parsedMsg) {
    ;
}

void handleUSER(Client& client, Server& server, Command& parsedMsg) {
    ;
}
