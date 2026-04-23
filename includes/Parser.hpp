#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>

/*
 * Parser
 * Used to parse raw IRC messages.
 * Splits a line into prefix, command and parameters.
 * Handles trailing parameter starting with ':'.
 * Does not validate commands, only extracts data.
 */

/*
 * Command
 * Represents a parsed IRC message.
 */

struct Command
{
    std::string prefix;
    std::string command;
    std::vector<std::string> params;
};

class Parser
{
public:
    static Command parse(const std::string &line);
};

#endif