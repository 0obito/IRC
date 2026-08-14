#ifndef DISPATCHER_HPP
#define DISPATCHER_HPP

#include <map>
#include <string>

/*
 * commandDispatcher class
 * It has a map with all command handlers.
 * It uses routeCommand to look up the passed command and call
 *  its corresponding "handler" function, if found ofc.
 */

class  Server;
class  Client;
struct Command;

typedef void (*commandHandler)(Server& server, Client& client, Command& parsedMsg);
typedef void (*commandBotHandler)(int socketfd, std::string target, Command cmd);

class commandDispatcher {
    private:
        std::map<std::string, commandHandler>       _handlers;
        std::map<std::string, commandBotHandler>    _BotHandlers;
    public:
        commandDispatcher();
        commandDispatcher(std::string Bot);
        ~commandDispatcher();
        void routeCommand(Server& server, Client& client, Command& parsedMsg);
        void botCommand(int socketfd, std::string target, Command cmd);
};

#endif
