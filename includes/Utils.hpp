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
bool        sendToClient(Server& server, Client& client, const std::string& command, std::string& target, std::string& messageText);
bool        broadcastToChannel(Server& server, Client& client, const std::string& command, std::string& target, std::string& messageText);

#endif