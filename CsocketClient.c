#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

int	main() {
    int clientFD;
    struct sockaddr_in  servAddress;
    int                 connectRet;
    char                server_request[2000];

    clientFD = socket(AF_INET, SOCK_STREAM, 0);
    if (clientFD < 0)
        perror("socket()");
    servAddress.sin_family = AF_INET;
    servAddress.sin_addr.s_addr = (1 << 24) + 127;
    servAddress.sin_port = htons(6969);
    connectRet = connect(clientFD, (const struct sockaddr *)&servAddress, sizeof(servAddress));
    if (connectRet != 0)
        perror("connect()");

    char    *cli_msg_one = "[CLIENT] I'm alive!\n";

    char    *cli_msg_two = "Hey there Mr Server!\n";
    char    *sent = "[CLIENT] message sent.\n";
    char    *received = "[CLIENT] message received.\n";
    char    *extra = "[CLIENT] from [SERVER]: ";
    char    *done = "[CLIENT] ---- Client is out! ----\n";

    memset(server_request, 0, sizeof(server_request));

    write(1, cli_msg_one, strlen(cli_msg_one));

    write(clientFD, cli_msg_two, strlen(cli_msg_two));
    write(1, sent, strlen(sent));

    ssize_t readdd= read(clientFD, server_request, sizeof(server_request));
    write(1, received, strlen(received));
    write(1, extra, strlen(extra));
    write(1, server_request, readdd/*strlen(server_request*/);

    close(clientFD);
    write(1, done, strlen(done));
    return (0);
}