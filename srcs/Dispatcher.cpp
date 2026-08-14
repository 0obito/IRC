#include "../includes/Dispatcher.hpp"
#include "../includes/DispatcherUtils.hpp"
#include "../includes/Utils.hpp"

#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Parser.hpp"

commandDispatcher::commandDispatcher() {
    _handlers["PASS"] = &handlePASS;
    _handlers["NICK"] = &handleNICK;
    _handlers["USER"] = &handleUSER;
    _handlers["PRIVMSG"] = &handlePRIVMSG;
    _handlers["PING"] = &handlePING;
    _handlers["PONG"] = &handlePONG;
    _handlers["JOIN"] = &handleJOIN;
    _handlers["PART"] = &handlePART;
    _handlers["KICK"] = &handleKICK;
    _handlers["TOPIC"] = &handleTOPIC;
    _handlers["INVITE"] = &handleINVITE;
    _handlers["MODE"] = &handleMODE;
}

commandDispatcher::commandDispatcher(std::string Bot) {
    (void)Bot;
    _BotHandlers["JOKE"] = &handleJOKE;
    _BotHandlers["ANONYM"] = &handleANONYM;
}
commandDispatcher::~commandDispatcher() {
}

void commandDispatcher::routeCommand(Server& server, Client& client, Command& parsedMsg) {
    if (_handlers.find(parsedMsg.command) != _handlers.end()) {
        commandHandler func = _handlers[parsedMsg.command];
        func(server, client, parsedMsg);
    }
    else {
        if (!client.isRegistered()) {
            return ;
        }
        std::string targetNick = client.getNick().empty() ? "*" : client.getNick();
        std::string serverName = server.getServerName();
        std::string reply;

        reply = makeReply(serverName, 421, targetNick, "Unknown command", parsedMsg.command);
        client.getSendQueue() += reply;
    }
}

void commandDispatcher::botCommand(int socketfd, std::string target, Command cmd) {
    if (_BotHandlers.find(cmd.command) != _BotHandlers.end()) {
        commandBotHandler func = _BotHandlers[cmd.command];
        func(socketfd, target, cmd);
    }
    else {
        std::string reply = "PRIVMSG " + target + "Unknown command" + cmd.command + "\r\n";
        send(socketfd, reply.c_str(), reply.length(), 0);
    }
}