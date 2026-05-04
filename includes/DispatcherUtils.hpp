#ifndef DISPATCHER_UTILS_HPP
#define DISPATCHER_UTILS_HPP

#include <string>
#include "includes/Server.hpp"
#include "includes/Client.hpp"
#include "includes/Parser.hpp"
#include "includes/Dispatcher.hpp"

void handlePASS(Client& client, Server& server, Command& parsedMsg);
void handleNICK(Client& client, Server& server, Command& parsedMsg);
void handleUSER(Client& client, Server& server, Command& parsedMsg);

#endif
