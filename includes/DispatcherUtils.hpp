#ifndef DISPATCHER_UTILS_HPP
#define DISPATCHER_UTILS_HPP

#include <string>
#include <sstream>
#include <iostream>
#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Parser.hpp"
#include "../includes/Dispatcher.hpp"

void handleCAP(Server& server, Client& client, Command& parsedMsg);
void handlePASS(Server& server, Client& client, Command& parsedMsg);
void handleNICK(Server& server, Client& client, Command& parsedMsg);
void handleUSER(Server& server, Client& client, Command& parsedMsg);
void handlePRIVMSG(Server& server, Client& client, Command& parsedMsg);

#endif
