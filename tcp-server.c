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
#include "x-common.h"


void get_file(int clientFd) {
    char* message;
    char* fName;

    message = calloc(INPUT_MAX, sizeof(char));
    fName = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL || fName == NULL) {
        return;
    }

    sprintf(message, "%s", "ready");
    if (send(clientFd, message, INPUT_MAX, 0) == - 1) {
        printf("tcp-server: failed to send get file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    if (recv(clientFd, fName, INPUT_MAX, 0) == -1) {
        printf("tcp-server: failed to receive get file name\n");
        return;
    }

    if (access(fName, F_OK)) {
        printf("tcp-server: file %s does not exist\n", fName);
        sprintf(message, "%s", "error");
        if (send(clientFd, message, INPUT_MAX, 0) == -1) {
            printf("tcp-server: failed to send file does not exist error\n");
        }
        return;
    }
    memset(message, 0, INPUT_MAX);

    sprintf(message, "%s", "ready");
    if (send(clientFd, message, INPUT_MAX, 0) == - 1) {
        printf("tcp-server: failed to send get file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    tcp_file_transmit("tcp-server", clientFd, fName);
}


void put_file(int clientFd)  {
    char* message;
    char* fName;

    message = calloc(INPUT_MAX, sizeof(char));
    fName = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL || fName == NULL) {
        printf("tcp-server: failed to allocate necessary memory\n");
        return;
    }

    sprintf(message, "%s", "ready");
    if (send(clientFd, message, INPUT_MAX, 0) == - 1) {
        printf("tcp-server: failed to send put file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    if (recv(clientFd, fName, INPUT_MAX, 0) == -1) {
        printf("tcp-server: failed to receive put file name\n");
        return;
    }

    if (!access(fName, F_OK)) {
        printf("tcp-server: file %s already exists\n", fName);
        sprintf(message, "%s", "error");
        if (send(clientFd, message, INPUT_MAX, 0) == -1) {
            printf("tcp-server: failed to send file exists error\n");
        }
        return;
    }
    memset(message, 0, INPUT_MAX);

    sprintf(message, "%s", "ready");
    if (send(clientFd, message, INPUT_MAX, 0) == -1) {
        printf("tcp-server: failed to send put file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    tcp_file_receive("tcp-server", clientFd, fName);
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
    int rResult;
    int qMax = 5;

    if (argc != 2) {
        printf("usage: ./tcp-server <port number>\n");
        exit(1);
    }

    port = argv[1];
    if (!check_port(port)) {
        printf("tcp-server: port number must be between 30000 and 40000\n");
        exit(1);
    }

    hName = calloc(INPUT_MAX, sizeof(char));
    if (hName == NULL) {
        printf("tcp-server: failed to allocate necessary memory\n");
        exit(1);
    }

    if (gethostname(hName, INPUT_MAX) == -1) {
        printf("tcp-server: failed to determine the name of the machine\n");
        exit(1);
    }
    if (!tcp_socket(&sockFd, &sockInfo, hName, port)) {
        printf("tcp-server: failed to create tcp socket for given host\n");
        exit(1);
    }

    if (bind(sockFd, sockInfo.ai_addr, sockInfo.ai_addrlen) == -1) {
        printf("tcp-server: failed to bind tcp socket for given host\n");
        exit(1);
    }
    if (listen(sockFd, qMax) == -1) {
        printf("tcp-server: failed to listen tcp socket for given host\n");
        exit(1);
    }

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        printf("tcp-server: failed to allocate necessary memory\n");
        exit(1);
    }

    while (1) {
        clientLen = sizeof(clientAddr);
        clientFd = accept(sockFd, &clientAddr, &clientLen);
        if (clientFd == -1) {
            printf("tcp-server: failed to accept incoming connection on socket\n");
            exit(1);
        }

        while (1) {
            rResult = recv(clientFd, message, INPUT_MAX, 0);
            if (rResult == 0) {
                break;
            }
            if (rResult == -1) {
                printf("tcp-server: failed to receive command from client\n");
                exit(1);
            }

            if (strcmp(message, "get") == 0) {
                get_file(clientFd);
            }
            if (strcmp(message, "put") == 0) {
                put_file(clientFd);
            }
        }
    }

    close(clientFd);
    close(sockFd);
    exit(0);
}
