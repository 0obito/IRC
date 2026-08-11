#ifndef DISPATCHER_HPP
#define DISPATCHER_HPP

#include <map>
#include <string>

class  Server;
class  Client;
struct Command;

typedef void (*commandHandler)(Server& server, Client& client, Command& parsedMsg);

class commandDispatcher {
    private:
        std::map<std::string, commandHandler> _handlers;

    public:
        commandDispatcher();
        ~commandDispatcher();
        void routeCommand(Server& server, Client& client, Command& parsedMsg);
};

#endif
