#include "../includes/IrcBot.hpp"

IrcBot::IrcBot(const std::string& ip, int port, const std::string& pass)
    : server_ip(ip),
    password(pass),
    server_port(port),
    socket_fd(-1)
{
    std::srand(std::time(0));
}

IrcBot::~IrcBot() {
    if (socket_fd != -1) {
        close(socket_fd);
    }
}

void IrcBot::connectToServer() {
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        throw std::runtime_error("ERROR: Couldn't create socket");
    }
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    in_addr_t servAddress = inet_addr(server_ip.c_str());
    if (servAddress == (in_addr_t)(-1)) {
        throw std::runtime_error("ERROR: Address is invalid / unsupported");
    }
    server_addr.sin_addr.s_addr = servAddress;
    std::cout << "Connecting to " << servAddress << ":" << server_port << "..." << std::endl;
    if (connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        throw std::runtime_error("ERROR: Connection failed");
    }
    std::cout << "Connected successfully!" << std::endl;
    return ;
}

void IrcBot::authenticate() {
    std::stringstream ss;

    std::cout << "bot attempting to register..." << std::endl;
    ss << "PASS :" << password << "\r\n";
    ss << "NICK bot\r\n";
    ss << "USER b 0 * B\r\n";

    std::string auth_msg = ss.str();
    send(socket_fd, auth_msg.c_str(), auth_msg.length(), 0);
}


void IrcBot::run() {
    char read_buffer[1024];
    std::string message_buffer = "";

    while (true) {
        std::memset(read_buffer, 0, sizeof(read_buffer));

        int bytes_received = recv(socket_fd, read_buffer, sizeof(read_buffer), 0);

        if (bytes_received <= 0) {
            throw std::runtime_error("ERROR: Couldn't read from the server");
        }

        message_buffer.append(read_buffer, bytes_received);

        size_t pos;
        while ((pos = message_buffer.find("\r\n")) != std::string::npos) {
            std::string complete_message = message_buffer.substr(0, pos);
            message_buffer.erase(0, pos + 2);

            std::cout << "message: " << complete_message << std::endl;
            handleMessage(complete_message);
        }
    }
}


void IrcBot::handleMessage(std::string& message) {
    Command command;
    Command cmd;
    commandDispatcher dispatcher("bot");

    if (message.compare(0, 5, "PING ") == 0) {
        std::string pong = "PONG " + message.substr(5) + "\r\n";
        send(socket_fd, pong.c_str(), pong.length(), 0);
        return;
    }
    else {
        command = Parser::parse(message);
        size_t pos = command.prefix.find("!");
        if (pos != std::string::npos) {
            std::string target = command.prefix.substr(0, pos);
            cmd = Parser::parse(command.params[1]);
            std::cout << "target: " << target << std::endl;
            dispatcher.botCommand(socket_fd, target, cmd);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "ERROR: Incorrect format. Try:" << std::endl;
        std::cerr << argv[0] << " <server_ip> <port> <password>" << std::endl;
        return 1;
    }
    std::string ip = argv[1];
    int port = std::atoi(argv[2]);
    std::string password = argv[3];

    if (port <= 0 || port > 65535) {
        std::cerr << "ERROR: Port must be between 1 and 65535." << std::endl;
        return 1;
    }

    try {
        IrcBot myBot(ip, port, password);
        myBot.connectToServer();
        myBot.authenticate();
        myBot.run();
    }
    catch (std::exception& e) {
        std::cerr<<e.what()<<std::endl;
    }
    return 0;
}
