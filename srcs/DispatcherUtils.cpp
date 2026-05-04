#include "includes/Dispatcher.hpp"
#include "includes/DispatcherUtils.hpp"

void handlePASS(Server& server, Client& client, Command& parsedMsg) {
    if (client.isRegistered) {
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

void handleNICK(Server& server, Client& client, Command& parsedMsg) {
    ;
}

void handleUSER(Server& server, Client& client, Command& parsedMsg) {
    ;
}
