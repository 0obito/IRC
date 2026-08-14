#include "../includes/IrcBot.hpp"

IrcBot::~IrcBot() {
    if (socket_fd != -1) {
        close(socket_fd);
    }
}

bool IrcBot::connectToServer() {
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        std::cerr << "Error: Could not create socket." << std::endl;
        return false;
    }
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Error: Invalid address / Address not supported." << std::endl;
        return false;
    }
    std::cout << "Connecting to " << server_ip << ":" << server_port << "..." << std::endl;
    if (connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error: Connection failed." << std::endl;
        return false;
    }
    std::cout << "Connected successfully!" << std::endl;
    return true;
}

void IrcBot::authenticate() {
    std::string auth_msg = "";
    
    if (!password.empty()) {
        auth_msg += "PASS " + password + "\r\n";
    }
    auth_msg += "NICK Bot\r\n";
    auth_msg += "USER b b b Bot\r\n";
    send(socket_fd, auth_msg.c_str(), auth_msg.length(), 0);
    std::cout << "Authentication sent." << std::endl;
}

void IrcBot::run() {
    char read_buffer[512]; 
    std::string message_buffer = "";
    while (true) {
        std::memset(read_buffer, 0, sizeof(read_buffer));
        int bytes_received = recv(socket_fd, read_buffer, sizeof(read_buffer) - 1, 0);
        if (bytes_received == 0) {
            std::cout << "diconnected from Server." << std::endl;
            break;
        } else if (bytes_received < 0) {
            std::cerr << "Error reading from Server." << std::endl;
            break;
        }
        message_buffer += read_buffer;
        size_t pos;
        while ((pos = message_buffer.find("\r\n")) != std::string::npos) {
            std::string complete_message = message_buffer.substr(0, pos);
            message_buffer.erase(0, pos + 2);
            std::cout << "message: " << complete_message << std::endl;
            handleMessage(complete_message);
        }
    }
}

void IrcBot::handleMessage(std::string message) {
    // Handle PING 
    Command command ;
    Command cmd;
    if (message.compare(0, 5, "PING ") == 0) {
        std::string pong = "PONG " + message.substr(5) + "\r\n";
        send(socket_fd, pong.c_str(), pong.length(), 0);
        return;
    }
    else{
        command = Parser::parse(message);
        if (command.command == "PRIVMSG"){
            cmd = Parser::parse(command.params[1]);
            //print command message
            std::cout << "command: " << std::endl;
            std::cout << "prefix: " << cmd.prefix << std::endl;
            std::cout << "command: " << cmd.command << std::endl;
            std::cout << "size of params: " << cmd.params.size() << std::endl;
            for(size_t i = 0; i < cmd.params.size(); i++)
                std::cout << "param: " << cmd.params[i] << std::endl;
        }
        //print command message
        // std::cout << "command: " << std::endl;
        // std::cout << "prefix: " << command.prefix << std::endl;
        // std::cout << "command: " << command.command << std::endl;
        // std::cout << "size of params: " << command.params.size() << std::endl;
        // for(size_t i = 0;i < command.params.size(); i++)
        //     std::cout << "param: " << command.params[i] << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "[ERROR] Incorrect format. Try:" << std::endl;
        std::cerr << argv[0] << " <server_ip> <port> <password>" << std::endl;
        return 1;
    }
    std::string ip = argv[1];
    int port = std::atoi(argv[2]);
    std::string password = argv[3];
    if (port <= 0 || port > 65535) {
        std::cerr << "Error: Port must be between 1 and 65535." << std::endl;
        return 1;
    }
    IrcBot myBot(ip, port, password);
    if (myBot.connectToServer()) {
        myBot.authenticate();
        myBot.run();
    }
    return 0;
}
