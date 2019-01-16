/* Nickolas Gough, nvg081, 11181823 */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#include "x-sockets.h"
#include "x-common.h"


int main(int argc, char* argv[]) {
    char* message;
    char* sName;
    char* sPort;
    int serverFd;
    struct addrinfo serverInfo;

    if (argc != 3) {
        printf("usage: ./x-client <host name> <port number>\n");
        exit(1);
    }

    sName = argv[1];
    sPort = argv[2];
    if (!check_port(sPort)) {
        printf("x-client: port number must be between 30000 and 40000\n");
        exit(1);
    }

    if (!udp_socket(&serverFd, &serverInfo, sName, sPort)) {
        printf("x-client: failed to create udp socket for given host\n");
        exit(1);
    }

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        printf("x-client: failed to allocate necessary memory\n");
        exit(1);
    }

    if (sendto(serverFd, message, INPUT_MAX, 0, (struct sockaddr*) serverInfo.ai_addr, serverInfo.ai_addrlen) == -1) {
        printf("x-client: failed to send to server\n");
        exit(1);
    }
    if (recvfrom(serverFd, message, INPUT_MAX, 0, (struct sockaddr*) NULL, NULL) == -1) {
        printf("x-client: failed to receive from server\n");
        exit(1);
    }

    close(serverFd);
    exit(0);
}
