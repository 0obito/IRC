#ifndef DISPATCHER_UTILS_HPP
#define DISPATCHER_UTILS_HPP

#include <string>
#include <sstream>
#include <iostream>
#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Parser.hpp"
#include "../includes/Dispatcher.hpp"

void handleCAP(Server &server, Client &client, Command &parsedMsg);
void handlePASS(Server &server, Client &client, Command &parsedMsg);
void handleNICK(Server &server, Client &client, Command &parsedMsg);
void handleUSER(Server &server, Client &client, Command &parsedMsg);
void handlePRIVMSG(Server &server, Client &client, Command &parsedMsg);
void handlePING(Server &server, Client &client, Command &parsedMsg);
void handlePONG(Server &server, Client &client, Command &parsedMsg);
void handleNOTICE(Server &server, Client &client, Command &parsedMsg);
void handleJOIN(Server &server, Client &client, Command &parsedMsg);
void handlePART(Server &server, Client &client, Command &parsedMsg);
void handleKICK(Server &server, Client &client, Command &parsedMsg);

#endif /* DISPATCHER_UTILS_HPP */
