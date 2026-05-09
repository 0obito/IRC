#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

/*
 * Utils
 * Small helper functions
 */
std::string makeReply(const std::string &serverName, int code, const std::string &target, const std::string &msg, const std::string &extraArgs = "");
std::string toLower(const std::string &s);
std::string trim(const std::string &s);
std::string itostr(int n);
std::string toUpper(const std::string &s);

#endif