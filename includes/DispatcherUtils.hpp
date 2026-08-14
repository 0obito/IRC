#ifndef DISPATCHER_UTILS_HPP
#define DISPATCHER_UTILS_HPP

#include <string>
#include <sstream>
#include <iostream>
#include <cstring>

/*
 * dispatcherUtils
 * contains all "handlers" function signatures
 */

class  Server;
class  Client;
struct Command;

void handlePASS(Server &server, Client &client, Command &parsedMsg);
void handleNICK(Server &server, Client &client, Command &parsedMsg);
void handleUSER(Server &server, Client &client, Command &parsedMsg);
void handlePRIVMSG(Server &server, Client &client, Command &parsedMsg);
void handlePING(Server &server, Client &client, Command &parsedMsg);
void handlePONG(Server &server, Client &client, Command &parsedMsg);
void handleJOIN(Server &server, Client &client, Command &parsedMsg);
void handlePART(Server &server, Client &client, Command &parsedMsg);
void handleKICK(Server &server, Client &client, Command &parsedMsg);
void handleTOPIC(Server &server, Client &client, Command &parsedMsg);
void handleINVITE(Server &server, Client &client, Command &parsedMsg);
void handleMODE(Server& server, Client& client, Command& parsedMsg);

// Bot
void    handleJOKE(int socketfd, std::string target, Command cmd);
void    handleANONYM(int socketfd, std::string target, Command cmd);

#endif
