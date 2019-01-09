/* Nickolas Gough, nvg081, 11181823 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#include "x-sockets.h"


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
        printf("usage: tcp-server <port number>\n");
        exit(err);
    }
    if (err == 2) {
        printf("tcp-server: failed to allocate memory\n");
        exit(err);
    }
    if (err == 3) {
        printf("tcp-server: failed to determine hostname\n");
        exit(err);
    }
    if (err == 4) {
        printf("tcp-server: failed to open tcp socket\n");
        exit(err);
    }
    if (err == 5) {
        printf("tcp-server: failed to bind the socket\n");
        exit(err);
    }
    if (err == 6) {
        printf("tcp-server: failed to listen on the socket\n");
        exit(err);
    }
    if (err == 7) {
        printf("tcp-server: failed to accept a connection\n");
    }
    if (err == 8) {
        printf("tcp-server: failed to receive on connection\n");
    }
}


void put_file(int clientFd)  {
    char* message;
    FILE* fptr;
    char* fName;
    long int fSize;
    long int rSize;

    message = calloc(INPUT_MAX, sizeof(char));
    fName = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL || fName == NULL) {
        return;
    }

    sprintf(message, "%s", "ready");
    send(clientFd, message, INPUT_MAX, 0);
    memset(message, 0, INPUT_MAX);

    recv(clientFd, fName, INPUT_MAX, 0);

    sprintf(message, "%s", "ready");
    send(clientFd, message, INPUT_MAX, 0);
    memset(message, 0, INPUT_MAX);

    recv(clientFd, &fSize, sizeof(fSize), 0);

    fptr = fopen(fName, "a");
    if (fptr == NULL) {
        return;
    }
    while (fSize > 0) {
        rSize = recv(clientFd, message, INPUT_MAX, 0);
        fwrite(message, sizeof(char), INPUT_MAX, fptr);

        memset(message, 0, INPUT_MAX);
        fSize -= rSize;
        printf("Appended to the file\n");
    }
    printf("Finished writing to the file\n");

    fclose(fptr);
}


int main(int argc, char* argv[]) {
    char* hName;
    char* port;
    int sockFd;
    struct addrinfo sockInfo;
    int clientFd;
    struct sockaddr clientAddr;
    socklen_t clientLen;
    char* message;
    int qMax = 5;

    if (argc != 2) {
        handle_fault(1);
    }
    port = argv[1];
    if (!check_port(port)) {
        handle_fault(1);
    }

    hName = calloc(INPUT_MAX, sizeof(char));
    if (hName == NULL) {
        handle_fault(2);
    }
    if (gethostname(hName, INPUT_MAX) == -1) {
        handle_fault(3);
    }

    if (!socket_tcp(&sockFd, &sockInfo, hName, port)) {
        handle_fault(4);
    }
    if (bind(sockFd, sockInfo.ai_addr, sockInfo.ai_addrlen) == -1) {
        handle_fault(5);
    }

    if (listen(sockFd, qMax) == -1) {
        handle_fault(6);
    }
    clientLen = sizeof(clientAddr);
    clientFd = accept(sockFd, &clientAddr, &clientLen);
    if (clientFd == -1) {
        handle_fault(7);
    }

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        handle_fault(2);
    }
    while (1) {
        if (recv(clientFd, message, INPUT_MAX, 0) == -1) {
            handle_fault(8);
        }
        if (strcmp(message, "put") == 0) {
            put_file(clientFd);
        }
    }

    exit(0);
}
