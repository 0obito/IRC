#include "../includes/Utils.hpp"

std::string makeReply(const std::string &serverName, int code, const std::string &target, const std::string &msg, const std::string &extraArgs)
{
    std::stringstream ss;

    ss << ":" << serverName << " ";

    if (code < 100) ss << "0";
    if (code < 10)  ss << "0";
    ss << code;

    ss << " " << target;

    if (!extraArgs.empty()) {
        ss << " " << extraArgs;
    }

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

void    sendToClient(Server& server, Client& client, Command& parsedMsg, std::string& target, std::string& messageText) {
    int         targetFD = server.nicknameOwner(target);
    std::string senderNick = client.getNick().empty() ? "*" : client.getNick();
    std::string serverName = server.getServerName();
    std::string reply;

    if (targetFD == -1) {
        reply = makeReply(serverName, 401, senderNick, "No such nickname", target);
        client.getSendQueue() += reply;
        return ;
    }
    std::cout<<"we can't be here, we shouldn't be here!\n";
    std::map<int, Client>::iterator iter = server.getMap().find(targetFD);
    if (iter != server.getMap().end()) {
        reply = ":" + client.getNick() + "!" + client.getUser() + "@127.0.0.1 " 
                + parsedMsg.command + " " + iter->second.getNick() + " :" + messageText + "\r\n";
        iter->second.getSendQueue() += reply;
        struct epoll_event current_ev;
        memset(&current_ev, 0, sizeof(current_ev));
        current_ev.events = EPOLLOUT | EPOLLIN;
        current_ev.data.fd = iter->first;
        epoll_ctl(server.get_epfd(), EPOLL_CTL_MOD, iter->first, &current_ev);
    }
}

void    broadcastToChannel(Server& server, Client& client, Command& parsedMsg, std::string& target, std::string& messageText) {
    const Channel*  targetChannel = server.getChannel(target);
    std::string     senderNick = client.getNick().empty() ? "*" : client.getNick();
    std::string     senderUser = client.getUser().empty() ? "*" : client.getUser();
    std::string     serverName = server.getServerName();
    std::string     reply;

    // channel doesn't exist
    if (targetChannel == NULL) {
        reply = makeReply(serverName, 401, senderNick, "No such channel", target);
        client.getSendQueue() += reply;
        return ;
    }

    // sender is not a member of the channel
    if (!targetChannel->isMember(client.getFd())) {
        std::cout << "Indeed\n";
        reply = makeReply(serverName, 404, senderNick, "Cannot send to channel", target);
        client.getSendQueue() += reply;
        return ;
    }

    std::set<int>::iterator iter = targetChannel->getMembers().begin();
    for (; iter != targetChannel->getMembers().end(); iter++) {
        if (*iter == client.getFd()) {
            continue ;
        }
        reply = ":" + senderNick + "!" + senderUser + "@127.0.0.1 " 
                + parsedMsg.command + " " + target + " :" + messageText + "\r\n";
        Client& tempClient = server.getMap().find(*iter)->second;
        tempClient.getSendQueue() += reply;
        struct epoll_event current_ev;
        memset(&current_ev, 0, sizeof(current_ev));
        current_ev.events = EPOLLOUT | EPOLLIN;
        current_ev.data.fd = tempClient.getFd();
        epoll_ctl(server.get_epfd(), EPOLL_CTL_MOD, tempClient.getFd(), &current_ev);
    }
}
