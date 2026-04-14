#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    int                 sock_fd;
    struct sockaddr_in  sock_add;
    struct sockaddr_in  client_add;
    socklen_t           client_add_size;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
                // param 1: specifies what kind of addresses we're dealing with. AF_LOCAL for local IPC (on the same computer), AF_INET for IPC over the INTERNTET (IPv4).
                // param 2: specifies what type of communication will be used. SOCK_STREAM for a reliable connection-oriented byte stream, SOCK_DGRAM for connectionless, unreliable messages.
                // param 3: specifies a particular protocol to be used with the socket, 0 is for default protocol to the selected domain and type.
    if (sock_fd < 0)
        perror("socket()");
    sock_add.sin_family = AF_INET;
    sock_add.sin_port = htons(6969);
    sock_add.sin_addr = INADDR_ANY;
    if (bind(sock_fd, (const struct sockaddr *)&sock_add, sizeof(sock_add)) < 0)
        perror("bind()");
    if (listen(sock_fd, 1) < 0)
        perror("listen()");
    client_add_size = sizeof(client_add);
    if (accept(sock_fd, (struct sockaddr *)&client_add, &client_add_size) < 0)
        perror("accept()");
    return (0);
}
