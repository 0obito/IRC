#include "../includes/Dispatcher.hpp"
#include "../includes/DispatcherUtils.hpp"
#include "../includes/Utils.hpp"

commandDispatcher::commandDispatcher() {
    _handlers["CAP"]  = &handleCAP;
    _handlers["PASS"] = &handlePASS;
    _handlers["NICK"] = &handleNICK;
    _handlers["USER"] = &handleUSER;
    _handlers["PRIVMSG"] = &handlePRIVMSG;
    _handlers["PING"] = &handlePING;
    _handlers["PONG"] = &handlePONG;
    _handlers["NOTICE"] = &handleNOTICE;
    _handlers["JOIN"] = &handleJOIN;
    _handlers["PART"] = &handlePART;
}

commandDispatcher::commandDispatcher(const commandDispatcher& other) {
    this->_handlers = other._handlers;
}

commandDispatcher& commandDispatcher::operator=(const commandDispatcher& other) {
    if (this != &other) {
        this->_handlers = other._handlers;
    }
    return *this;
}

commandDispatcher::~commandDispatcher() {
}

void commandDispatcher::routeCommand(Server& server, Client& client, Command& parsedMsg) {
    if (_handlers.find(parsedMsg.command) != _handlers.end()) {
        commandHandler func = _handlers[parsedMsg.command];
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
        // std::cerr << "ERR_UNKNOWNCOMMAND (421)" << std::endl;
    }
}
