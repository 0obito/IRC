#include "Utils.hpp"
#include <sstream>
#include <cctype>

std::string makeReply(int code, const std::string &target, const std::string &msg)
{
    std::stringstream ss;

    ss << ":ircserv ";

    if (code < 100) ss << "0";
    if (code < 10)  ss << "0";
    ss << code;

    ss << " " << target;
    ss << " :" << msg << "\r\n";

    return ss.str();
}

std::string toLower(const std::string &s)
{
    std::string result = s;

    for (size_t i = 0; i < result.size(); i++)
        result[i] = std::tolower(static_cast<unsigned char>(result[i]));

    return result;
}

std::string trim(const std::string &s)
{
    size_t start = 0;
    while (start < s.size() &&
          (s[start] == ' ' || s[start] == '\t' || s[start] == '\r' || s[start] == '\n'))
        start++;

    if (start == s.size())
        return "";

    size_t end = s.size() - 1;
    while (end > start &&
          (s[end] == ' ' || s[end] == '\t' || s[end] == '\r' || s[end] == '\n'))
        end--;

    return s.substr(start, end - start + 1);
}

std::string itostr(int n)
{
    std::stringstream ss;
    ss << n;
    return ss.str();
}

std::string toUpper(const std::string &s)
{
    std::string res = s;
    for (size_t i = 0; i < res.size(); i++)
        res[i] = std::toupper(res[i]);
    return res;
}