#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <sstream>
#include <cctype>
#include "Server.hpp"
#include "Channel.hpp"
#include "Parser.hpp"

/*
 * Utils
 * Small helper functions
 */
std::string toLower(const std::string &s);
std::string trim(const std::string &s);
std::string itostr(int n);
std::string toUpper(const std::string &s);

std::string makeReply(const std::string &serverName, int code, const std::string &target, const std::string &msg, const std::string &extraArgs = "");
void        sendToClient(Server& server, Client& client, Command& parsedMsg, std::string& target, std::string& messageText);
void        broadcastToChannel(Server& server, Client& client, Command& parsedMsg, std::string& target, std::string& messageText);

#endif /* UTILS_HPP */
