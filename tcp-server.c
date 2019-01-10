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
    tcp_server_get("tcp-server", clientFd, NULL, 1);
}


void put_file(int clientFd)  {
    tcp_server_put("tcp-server", clientFd, NULL, 1);
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
    if (!socket_tcp(&sockFd, &sockInfo, hName, port)) {
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

    close(sockFd);
    exit(0);
}
