#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

/*
 * Utils
 * Small helper functions
 */

std::string makeReply(int code, const std::string &target, const std::string &msg);
std::string toLower(const std::string &s);
std::string trim(const std::string &s);
std::string itostr(int n);
std::string toUpper(const std::string &s);

#endif