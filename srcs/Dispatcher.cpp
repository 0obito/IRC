#include "includes/Dispatcher.hpp"
#include "includes/DispatcherUtils.hpp"

commandDispatcher::commandDispatcher() {
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

void commandDispatcher::routeCommand(Server& server, Client& client, Command &msg) {

    // expecting Nisrine to hand me the command separated (like: "PASS" or "NICK").
    // will ask her about the Parser class later, it doesn't look done yet.

    // i've changed the parser argument with the Command bcz it's the struct that contains the parts of the msg

    if (_handlers.find(msg.command) != _handlers.end()) {
        commandHandler func = _handlers[msg.command];
        func(server, client, msg);
    }
    else {
        std::cout << "Command not found!" << std::endl;

        // send the convenient numeric reply to the client!

    }
}
