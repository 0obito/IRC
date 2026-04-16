#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int	main() {
    int clientFD;
    struct sockaddr_in  servAddress;
    int                 connectRet;

    clientFD = socket(AF_INET, SOCK_STREAM, 0);
    if (clientFD < 0)
        perror("socket()");
    servAddress.sin_family = AF_INET;
    servAddress.sin_addr.s_addr = (1 << 24) + 127;
    servAddress.sin_port = htons(6969);
    connectRet = connect(clientFD, (const struct sockaddr *)&servAddress, sizeof(servAddress));
    if (connect != 0)
        perror("connect()");
    // Next on: Talk with server!
    write();
    return (0);
}
