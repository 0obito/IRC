#ifndef DISPATCHER_HPP
#define DISPATCHER_HPP

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include "Server.hpp"
#include "Client.hpp"
#include "DispatcherUtils.hpp"
#include "Parser.hpp"

typedef void (*commandHandler)(Server& server, Client& client, Command& parsedMsg);

class commandDispatcher {
    private:
        std::map<std::string, commandHandler> _handlers;
        std::map<std::string, commandHandler> _Boothandlers;

    public:
        commandDispatcher();
        commandDispatcher(const commandDispatcher& other);
        commandDispatcher& operator=(const commandDispatcher& other);
        ~commandDispatcher();

        void routeCommand(Server& server, Client& client, Command& parsedMsg);
        void handleBoot(Server& server, Client& client, Command& parsedMsg);
};

void    Boot(Server& server, Client& client, Command& parsedMsg);
void    handleBootHelpe(Server& server, Client& client, Command& parsedMsg);
void    handleBootUsers(Server& server, Client& client, Command& parsedMsg);
void    handleBootAnonyme(Server& server, Client& client, Command& parsedMsg);
void    handleBootRoles(Server& server, Client& client, Command& parsedMsg);
void    handleBootJoke(Server& server, Client& client, Command& parsedMsg);


#endif /* DISPATCHER_HPP */
