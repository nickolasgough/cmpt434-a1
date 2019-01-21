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


#define N_INPUTS 3


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

    /* Issue get request */
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

    /* Send file name */
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

    /* Receive the file */
    tcp_file_receive("x-client", serverFd, lFile);
}


void put_file(int serverFd, char* lFile, char* rFile) {
    char* message;

    if (access(lFile, F_OK)) {
        printf("x-client: file %s does not exist\n", lFile);
        return;
    }

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        printf("x-client: failed to allocate necessary memory\n");
        return;
    }

    /* Issue put request */
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

    /* Send file name */
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

    /* Transmit the file */
    tcp_file_transmit("x-client", serverFd, lFile);
}


int parse_cmd(char* src, char* del, char** dest) {
    int n = 0;
    char* token;

    src = strtok(src, "\n");

    token = strtok(src, del);
    if (token == NULL) {
        return 0;
    }
    do {
        if (n < 3) {
            dest[n] = token;
            n += 1;
        }
        token = strtok(NULL, del);
    } while (token != NULL);

    return n <= 3;
}


int main(int argc, char* argv[]) {
    char* command;
    char** inputs;
    char* action;
    char* lFile;
    char* rFile;
    char* sName;
    char* sPort;
    int serverFd;
    struct addrinfo* sockInfo;

    if (argc != 3) {
        printf("usage: ./x-client <host name> <port number>\n");
        exit(1);
    }

    /* Arguments and connect */
    sName = argv[1];
    sPort = argv[2];
    if (!check_port(sPort)) {
        printf("x-client: port number must be between 30000 and 40000\n");
        exit(1);
    }
    if (!tcp_socket(&serverFd, &sockInfo, sName, sPort)) {
        printf("x-client: failed to create tcp socket for given host\n");
        exit(1);
    }
    if (connect(serverFd, sockInfo->ai_addr, sockInfo->ai_addrlen) == -1) {
        printf("x-client: failed to connect tcp socket for given host\n");
        exit(1);
    }

    /* Interact with user */
    command = calloc(INPUT_MAX, sizeof(char));
    inputs = calloc(N_INPUTS, sizeof(char*));
    if (command == NULL || inputs == NULL) {
        printf("x-client: failed to allocate necessary memory\n");
        exit(1);
    }
    while (1) {
        printf("x-client? ");

        fgets(command, INPUT_MAX, stdin);
        if (parse_cmd(command, " ", inputs)) {
            action = inputs[0];
            lFile = inputs[1];
            rFile = inputs[2];

            if (strcmp(action, "quit") == 0) {
                exit(0);
            }
            if (strcmp(action, "get") == 0) {
                get_file(serverFd, lFile, rFile);
            }
            else if (strcmp(action, "put") == 0) {
                put_file(serverFd, lFile, rFile);
            }
            else {
                printf("Unknown command.\n");
                printf("Known commands:\n");
                printf("get <local file> <remote file>\n");
                printf("put <local file> <remote file>\n");
                printf("quit\n");
            }
        } else {
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
