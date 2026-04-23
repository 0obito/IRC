#include "Parser.hpp"
#include "Utils.hpp"
#include <sstream>

Command Parser::parse(const std::string &line)
{
    Command cmd;

    std::string clean = trim(line);
    std::stringstream ss(clean);
    std::string token;
    int paramCount = 0;

    ss >> token;
    if (!token.empty() && token[0] == ':')
    {
        cmd.prefix = token.substr(1);
        ss >> cmd.command;
        cmd.command = toUpper(token);
    }
    else
    {
        cmd.command = toUpper(token);
    }

    while (ss >> token)
    {
        if (paramCount >= 15)
            break;

        if (token[0] == ':')
        {
            std::string rest;
            std::getline(ss, rest);
            cmd.params.push_back(token.substr(1) + rest);
            break;
        }

        cmd.params.push_back(token);
        paramCount++;
    }

    return cmd;
}