#include "includes/Dispatcher.hpp"
#include "includes/DispatcherUtils.hpp"

commandDispatcher::commandDispatcher() {
    _handlers["CAP"]  = &handleCAP;
    _handlers["PASS"] = &handlePASS;
    _handlers["NICK"] = &handleNICK;
    _handlers["USER"] = &handleUSER;
}

commandDispatcher::commandDispatcher(const commandDispatcher& other) {
}

commandDispatcher& commandDispatcher::operator=(const commandDispatcher& other) {
}

commandDispatcher::~commandDispatcher() {
}

void commandDispatcher::routeCommand(Server& server, Client& client, Command& msg) {
    if (_handlers.find(msg.command) != _handlers.end()) {
        commandHandler func = _handlers[msg.command];
        func(server, client, msg);
    }
    else {
        std::cerr << "ERR_UNKNOWNCOMMAND (421)" << std::endl;
    }
}
