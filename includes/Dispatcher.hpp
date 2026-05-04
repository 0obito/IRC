#ifndef DISPATCHER_HPP
#define DISPATCHER_HPP

#include <vector>
#include <map>
#include <string>
#include <iostream>

typedef void (*commandHandler)(Server& server, Client& client, Parser& parser);

class commandDispatcher {
    private:
        std::map<std::string, commandHandler> _handlers;

    public:
        commandDispatcher();
        commandDispatcher(const commandDispatcher& other);
        commandDispatcher& operator=(const commandDispatcher& other);
        ~commandDispatcher();
        void routeCommand(Server& server, Client& client, Command &msg);
};

#endif
