#include "../includes/Dispatcher.hpp"
#include "../includes/DispatcherUtils.hpp"
#include "../includes/Utils.hpp"

void    Boot(Server& server, Client& client, Command& parsedMsg) {
    commandDispatcher BOOT;
    BOOT.handleBoot(server, client, parsedMsg);
}

void    commandDispatcher::handleBoot(Server& server, Client& client, Command& parsedMsg) {
     if (_Boothandlers.find(parsedMsg.command) != _Boothandlers.end()) {
        commandHandler func = _Boothandlers[parsedMsg.params[0]];
        func(server, client, parsedMsg);
    }
    else {
        if (!client.isRegistered()) {
            return ;
        }
        std::string targetNick = client.getNick().empty() ? "*" : client.getNick();
        std::string serverName = server.getServerName();
        std::string reply;

        reply = makeReply(serverName, 421, targetNick, "Unknown command", parsedMsg.command);
        client.getSendQueue() += reply;
    }
}
