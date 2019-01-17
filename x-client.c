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


void get_file(int serverFd, char* lFile, char* rFile) {
    char* message;

    if (!access(lFile, F_OK)) {
        printf("x-client: file %s already exists\n", lFile);
        return;
    }

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        printf("x-client: failed to allocate necessary memory\n");
        return;
    }

    sprintf(message, "%s", "get");
    if (send(serverFd, message, INPUT_MAX, 0) == -1) {
        printf("x-client: failed to transmit the get command\n");
        return;
    }
    if (recv(serverFd, message, INPUT_MAX, 0) == -1) {
        printf("x-client: failed to receive get command response\n");
        return;
    }
    if (strcmp(message, "ready") != 0) {
        printf("x-client: received unexpected get command response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    if (send(serverFd, rFile, INPUT_MAX, 0) == -1) {
        printf("x-client: failed to transmit the get file name\n");
        return;
    }
    if (recv(serverFd, message, INPUT_MAX, 0) == -1) {
        printf("x-client: failed to receive get file name response\n");
        return;
    }
    if (strcmp(message, "ready") != 0) {
        printf("x-client: received unexpected get file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    tcp_file_receive("x-client", serverFd, lFile);
}


void put_file(int serverFd, char* lFile, char* rFile) {
    char* message;
    FILE* fPtr;

    if (access(lFile, F_OK)) {
        printf("x-client: file %s does not exist\n", lFile);
        return;
    }

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        printf("x-client: failed to allocate necessary memory\n");
        return;
    }

    fPtr = fopen(lFile, "r");
    if (fPtr == NULL) {
        printf("x-client: failed to open file %s", lFile);
        return;
    }

    sprintf(message, "%s", "put");
    if (send(serverFd, message, INPUT_MAX, 0) == -1) {
        printf("x-client: failed to transmit the put command\n");
        return;
    }
    if (recv(serverFd, message, INPUT_MAX, 0) == -1) {
        printf("x-client: failed to receive put command response\n");
        return;
    }
    if (strcmp(message, "ready") != 0) {
        printf("x-client: received unexpected put command response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    if (send(serverFd, rFile, INPUT_MAX, 0) == -1) {
        printf("x-client: failed to transmit the put file name\n");
        return;
    }
    if (recv(serverFd, message, INPUT_MAX, 0) == -1) {
        printf("x-client: failed to receive put file name response\n");
        return;
    }
    if (strcmp(message, "ready") != 0) {
        printf("x-client: received unexpected put file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    tcp_file_transmit("x-client", serverFd, lFile);
}


int main(int argc, char* argv[]) {
    char* cmd;
    char* lFile;
    char* rFile;
    char* hName;
    char* port;
    int serverFd;
    struct addrinfo* sockInfo;

    if (argc != 3) {
        printf("usage: ./x-client <host name> <port number>\n");
        exit(1);
    }

    hName = argv[1];
    port = argv[2];
    if (!check_port(port)) {
        printf("x-client: port number must be between 30000 and 40000\n");
        exit(1);
    }

    if (!tcp_socket(&serverFd, &sockInfo, hName, port)) {
        printf("x-client: failed to create tcp socket for given host\n");
        exit(1);
    }
    if (connect(serverFd, sockInfo->ai_addr, sockInfo->ai_addrlen) == -1) {
        printf("x-client: failed to connect tcp socket for given host\n");
        exit(1);
    }

    cmd = calloc(INPUT_MAX, sizeof(char));
    lFile = calloc(INPUT_MAX, sizeof(char));
    rFile = calloc(INPUT_MAX, sizeof(char));
    if (cmd == NULL || rFile == NULL || lFile == NULL) {
        printf("x-client: failed to allocate necessary memory\n");
        exit(1);
    }

    while (1) {
        printf("x-client? ");

        scanf("%s", cmd);
        if (strcmp(cmd, "quit") == 0) {
            exit(0);
        }

        scanf("%s %s", lFile, rFile);
        if (strcmp(cmd, "get") == 0) {
            get_file(serverFd, lFile, rFile);
        }
        else if (strcmp(cmd, "put") == 0) {
            put_file(serverFd, lFile, rFile);
        }
        else {
            printf("Unknown command.\n");
            printf("Known commands:\n");
            printf("get <local file> <remote file>\n");
            printf("put <local file> <remote file>\n");
            printf("quit\n");
        }
    }

    close(serverFd);
    exit(0);
}
