/* Nickolas Gough, nvg081, 11181823 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "x-sockets.h"


#define STD_IN 0

#define PORT_MIN 30000
#define PORT_MAX 40000

#define INPUT_MAX 1000


typedef struct message_t {
    int action;
    char* lFile;
    char* rFile;
    char* part;
}


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


int main(int argc, char* argv[]) {
    char* hName;
    char* port;
    int sockFd;
    struct addrinfo sockInfo;
    int clientFd;
    struct sockaddr clientAddr;
    socklen_t clientLen;
    message_t msg;
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

    if (buffer == NULL) {
        handle_fault(2);
    }
    while (1) {
        if (recv(clientFd, msg, sizeof(message_t)) == -1) {
            handle_fault(8);
        }
    }

    exit(0);
}
