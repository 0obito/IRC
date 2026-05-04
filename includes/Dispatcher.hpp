#ifndef DISPATCHER_HPP
#define DISPATCHER_HPP

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include "includes/Parser.hpp"  // to recognize command structure.

typedef void (*commandHandler)(Server& server, Client& client, Command& parsedMsg);

class commandDispatcher {
    private:
        std::map<std::string, commandHandler> _handlers;

    public:
        commandDispatcher();
        commandDispatcher(const commandDispatcher& other);
        commandDispatcher& operator=(const commandDispatcher& other);
        ~commandDispatcher();
        void routeCommand(Server& server, Client& client, Command& parsedMsg);
};

#endif
