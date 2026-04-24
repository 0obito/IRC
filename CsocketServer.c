#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

int main() {
    int                 sock_fd;
    struct sockaddr_in  sock_add;
    struct sockaddr_in  client_add;
    socklen_t           client_add_size;
    int                 client_fd;
    char                client_request[2000];

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
                // param 1: specifies what kind of addresses we're dealing with. AF_LOCAL for local IPC (on the same computer), AF_INET for IPC over the INTERNTET (IPv4).
                // param 2: specifies what type of communication will be used. SOCK_STREAM for a reliable connection-oriented byte stream, SOCK_DGRAM for connectionless, unreliable messages.
                // param 3: specifies a particular protocol to be used with the socket, 0 is for default protocol to the selected domain and type.
    if (sock_fd < 0)
        perror("socket()");
    int opt = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        perror("setsockopt()");
    sock_add.sin_family = AF_INET;
    sock_add.sin_port = htons(6969);
    sock_add.sin_addr.s_addr = INADDR_ANY;
    if (bind(sock_fd, (const struct sockaddr *)&sock_add, sizeof(sock_add)) < 0)
        perror("bind()");
    if (listen(sock_fd, 1) < 0)
        perror("listen()");
    client_add_size = sizeof(client_add);
    client_fd = accept(sock_fd, (struct sockaddr *)&client_add, &client_add_size);
    if (client_fd < 0)
        perror("accept()");

    char *srv_msg_one = "[SERVER] I'm alive!\n";

    char *srv_msg_two = "I got you Mr Client! How can I help you today?\n";
    char    *sent = "[SERVER] message sent.\n";
    char    *received = "[SERVER] message received.\n";
    char    *extra = "[SERVER] from [CLIENT]: ";
    char    *done = "[SERVER] ---- Server is out! ----\n";

    memset(client_request, 0, sizeof(client_request));

    write(1, srv_msg_one, strlen(srv_msg_one));

    int readdd = read(client_fd, client_request, sizeof(client_request));
    write(1, received, strlen(received));
    write(1, extra, strlen(extra));
    write(1, client_request, readdd/*sizeof(client_request)*/);

    write(client_fd, srv_msg_two, strlen(srv_msg_two));
    write(1, sent, strlen(sent));

    close(client_fd);
    close(sock_fd);
    write(1, done, strlen(done));
    return (0);
}