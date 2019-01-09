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

void handle_fault(int err) {
    if (err == 1) {
        printf("usage: x-client <host name> <port number>\n");
        exit(err);
    }
    if (err == 2) {
        printf("x-client: failed to allocate memory\n");
        exit(err);
    }
    if (err == 3) {
        printf("x-client: failed to read command\n");
    }
    if (err == 4) {
        printf("x-client: failed to put the file\n");
    }
    if (err == 5) {
        printf("x-client: the file does not exist\n");
    }
    if (err == 6) {
        printf("x-client: failed to open tcp socket\n");
        exit(err);
    }
    if (err == 7) {
        printf("x-client: failed to connect on socket\n");
        exit(err);
    }
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
        return;
    }
    fptr = fopen(lFile, "r");
    if (fptr == NULL) {
        return;
    }

    message = calloc(INPUT_MAX, sizeof(char));
    fPart = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL || fPart == NULL) {
        return;
    }

    sprintf(message, "%s", "put");
    send(sockFd, message, INPUT_MAX, 0);
    recv(sockFd, message, INPUT_MAX, 0);
    strcmp(message, "ready");

    memset(message, 0, INPUT_MAX);

    send(sockFd, rFile, INPUT_MAX, 0);
    recv(sockFd, message, INPUT_MAX, 0);
    strcmp(message, "ready");

    memset(message, 0, INPUT_MAX);

    fseek(fptr, 0, SEEK_END);
    fSize = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);
    send(sockFd, &fSize, sizeof(fSize), 0);
    recv(sockFd, message, INPUT_MAX, 0);
    strcmp(message, "ready");

    memset(message, 0, INPUT_MAX);

    printf("Transmitting the file\n");
    while (fread(fPart, sizeof(char), INPUT_MAX, fptr) > 0) {
        printf("Sending %s\n of size %ld", fPart, strlen(fPart));
        if (send(sockFd, fPart, INPUT_MAX, 0) == -1) {
            printf("Failed to send\n");
            return;
        }
        memset(fPart, 0, INPUT_MAX);
    }
    printf("Finished sending the file\n");

    fclose(fptr);
    return;
}


int main(int argc, char* argv[]) {
    char cmd[INPUT_MAX];
    char lFile[INPUT_MAX];
    char rFile[INPUT_MAX];
    char* hName;
    char* port;
    int sockFd;
    struct addrinfo sockInfo;

    if (argc != 3) {
        handle_fault(1);
    }

    hName = argv[1];
    port = argv[2];
    if (!check_port(port)) {
        handle_fault(1);
    }

    if (!socket_tcp(&sockFd, &sockInfo, hName, port)) {
        handle_fault(6);
    }
    if (connect(sockFd, sockInfo.ai_addr, sockInfo.ai_addrlen) == -1) {
        handle_fault(7);
    }

    while (1) {
        printf("x-client ? ");
        if (scanf("%s", cmd) < 0) {
            handle_fault(3);
        }
        if (strcmp(cmd, "quit") == 0) {
            exit(0);
        }
        if (scanf("%s %s", lFile, rFile) < 0) {
            handle_fault(3);
        }
        
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
