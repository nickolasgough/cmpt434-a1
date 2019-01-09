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
#include "x-files.h"


#define STD_IN 0

#define PORT_MIN 30000
#define PORT_MAX 40000

#define INPUT_MAX 1000


int check_port(char* p) {
    if (atoi(p) < PORT_MIN) {
        return 0;
    }
    if (atoi(p) > PORT_MAX) {
        return 0;
    }

    return 1;
}


void get_file(int sockFd, char* lFile, char* rFile) {
    printf("Getting file: %s, %s\n", lFile, rFile);
}


void put_file(int sockFd, char* lFile, char* rFile) {
    FILE* fptr;
    long int fSize;
    char* fPart;
    char* message;

    if (access(lFile, F_OK)) {
        printf("x-client: file %s does not exist\n", lFile);
        return;
    }

    fptr = fopen(lFile, "r");
    if (fptr == NULL) {
        printf("x-client: failed to open file %s", lFile);
        return;
    }

    message = calloc(INPUT_MAX, sizeof(char));
    fPart = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL || fPart == NULL) {
        printf("x-client: failed to allocate necessary memory\n");
        return;
    }

    sprintf(message, "%s", "put");
    if (send(sockFd, message, INPUT_MAX, 0) == -1) {
        printf("x-client: failed to transmit the put command\n");
        return;
    }
    if (recv(sockFd, message, INPUT_MAX, 0) == -1) {
        printf("x-client: failed to receive put command response\n");
        return;
    }
    if (strcmp(message, "ready") != 0) {
        printf("x-client: received unexpected put command response\n");
        return;
    }

    memset(message, 0, INPUT_MAX);

    if (send(sockFd, rFile, INPUT_MAX, 0) == -1) {
        printf("x-client: failed to transmit the file name\n");
        return;
    }
    if (recv(sockFd, message, INPUT_MAX, 0) == -1) {
        printf("x-client: failed to receive put file name response\n");
        return;
    }
    if (strcmp(message, "ready") != 0) {
        printf("x-client: received unexpected put file name response\n");
        return;
    }

    memset(message, 0, INPUT_MAX);

    fseek(fptr, 0, SEEK_END);
    fSize = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);

    if (send(sockFd, &fSize, sizeof(fSize), 0) == -1) {
        printf("x-client: failed to transmit the file size\n");
        return;
    }
    if (recv(sockFd, message, INPUT_MAX, 0) == -1) {
        printf("x-client: failed to receive put file size response\n");
        return;
    }
    if (strcmp(message, "ready") != 0) {
        printf("x-client: received unexpected put file size response\n");
        return;
    }

    memset(message, 0, INPUT_MAX);

    printf("x-client: transmitting the file...\n");
    while (fread(fPart, sizeof(char), INPUT_MAX, fptr) > 0) {
        if (send(sockFd, fPart, INPUT_MAX, 0) == -1) {
            printf("x-client: failed to put the whole file\n");
            return;
        }

        memset(fPart, 0, INPUT_MAX);
    }
    printf("x-client: file successfully transmitted\n");

    fclose(fptr);
}


int main(int argc, char* argv[]) {
    char cmd[INPUT_MAX];
    char* lFile;
    char* rFile;
    char* hName;
    char* port;
    int sockFd;
    struct addrinfo sockInfo;

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

    if (!socket_tcp(&sockFd, &sockInfo, hName, port)) {
        printf("x-client: failed to create tcp socket for given host\n");
        exit(1);
    }
    if (connect(sockFd, sockInfo.ai_addr, sockInfo.ai_addrlen) == -1) {
        printf("x-client: failed to connect tcp socket for given host\n");
        exit(1);
    }

    lFile = calloc(INPUT_MAX, sizeof(char));
    rFile = calloc(INPUT_MAX, sizeof(char));
    if (rFile == NULL || lFile == NULL) {
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
            get_file(sockFd, lFile, rFile);
        }
        else if (strcmp(cmd, "put") == 0) {
            put_file(sockFd, lFile, rFile);
        }
        else {
            printf("Unknown command.\n");
            printf("Known commands:\n");
            printf("get <local file> <remote file>\n");
            printf("put <local file> <remote file>\n");
            printf("quit\n");
        }
    }

    close(sockFd);
    exit(0);
}
