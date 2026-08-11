#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <sstream>

/*
 * Utils
 * Small helper functions
 */

class  Server;
class  Client;
struct Command;

std::string toLower(const std::string &s);
std::string trim(const std::string &s);
std::string itostr(int n);
std::string toUpper(const std::string &s);

std::string makeReply(const std::string &serverName, int code, const std::string &target, const std::string &msg, const std::string &extraArgs = "");
void        sendToClient(Server& server, Client& client, Command& parsedMsg, std::string& target, std::string& messageText);
void        broadcastToChannel(Server& server, Client& client, Command& parsedMsg, std::string& target, std::string& messageText);

#endif