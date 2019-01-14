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


void get_file(int serverFd, struct sockaddr clientAddr, socklen_t clientLen) {
    char* message;
    char* fName;

    message = calloc(INPUT_MAX, sizeof(char));
    fName = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL || fName == NULL) {
        return;
    }

    printf("Sending to proxy...\n");
    sprintf(message, "%s", "ready");
    if (sendto(serverFd, message, INPUT_MAX, MSG_CONFIRM, &clientAddr, clientLen) == - 1) {
        printf("udp-server: failed to send get file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    printf("Receiving from proxy...\n");
    if (recvfrom(serverFd, fName, INPUT_MAX, 0, &clientAddr, &clientLen) == -1) {
        printf("udp-server: failed to receive get file name\n");
        return;
    }

    if (access(fName, F_OK)) {
        printf("udp-server: file %s does not exist\n", fName);
        sprintf(message, "%s", "error");
        if (sendto(serverFd, message, INPUT_MAX, 0, &clientAddr, clientLen) == -1) {
            printf("udp-server: failed to send file does not exist error\n");
        }
        return;
    }
    memset(message, 0, INPUT_MAX);

    sprintf(message, "%s", "ready");
    if (sendto(serverFd, message, INPUT_MAX, 0, &clientAddr, clientLen) == -1) {
        printf("udp-server: failed to send get file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    udp_file_transmit("udp-server", serverFd, fName, clientAddr, clientLen);
}


void put_file(int serverFd, struct sockaddr clientAddr, socklen_t clientLen)  {
    char* message;
    char* fName;

    message = calloc(INPUT_MAX, sizeof(char));
    fName = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL || fName == NULL) {
        printf("udp-server: failed to allocate necessary memory\n");
        return;
    }

    sprintf(message, "%s", "ready");
    if (sendto(serverFd, message, INPUT_MAX, 0, &clientAddr, clientLen) == - 1) {
        printf("udp-server: failed to send put file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    if (recvfrom(serverFd, fName, INPUT_MAX, 0, &clientAddr, &clientLen) == -1) {
        printf("udp-server: failed to receive put file name\n");
        return;
    }

    if (!access(fName, F_OK)) {
        printf("udp-server: file %s already exists\n", fName);
        sprintf(message, "%s", "error");
        if (sendto(serverFd, message, INPUT_MAX, 0, &clientAddr, clientLen) == -1) {
            printf("udp-server: failed to send file exists error\n");
        }
        return;
    }
    memset(message, 0, INPUT_MAX);

    sprintf(message, "%s", "ready");
    if (sendto(serverFd, message, INPUT_MAX, 0, &clientAddr, clientLen) == - 1) {
        printf("udp-server: failed to send put file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    udp_file_receive("udp-server", serverFd, fName, clientAddr, clientLen);
}


int main(int argc, char* argv[]) {
    char* hName;
    char* port;
    int serverFd;
    struct addrinfo serverInfo;
    struct sockaddr clientAddr;
    socklen_t clientLen;
    char* message;
    int rSize;

    if (argc != 2) {
        printf("usage: ./udp-server <port number>\n");
        exit(1);
    }

    port = argv[1];
    if (!check_port(port)) {
        printf("udp-server: port number must be between 30000 and 40000\n");
        exit(1);
    }

    hName = calloc(INPUT_MAX, sizeof(char));
    if (hName == NULL) {
        printf("udp-server: failed to allocate necessary memory\n");
        exit(1);
    }

    if (gethostname(hName, INPUT_MAX) == -1) {
        printf("udp-server: failed to determine the name of the machine\n");
        exit(1);
    }
    if (!udp_socket(&serverFd, &serverInfo, hName, port)) {
        printf("udp-server: failed to create tcp socket for given host\n");
        exit(1);
    }

    if (bind(serverFd, serverInfo.ai_addr, serverInfo.ai_addrlen) == -1) {
        printf("udp-server: failed to bind tcp socket for given host\n");
        exit(1);
    }

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        printf("udp-server: failed to allocate necessary memory\n");
        exit(1);
    }

    while (1) {
        memset(&clientAddr, 0, sizeof(clientAddr));

        rSize = recvfrom(serverFd, message, INPUT_MAX, MSG_WAITALL, &clientAddr, &clientLen);
        if (rSize == -1) {
            printf("udp-server: failed to receive command from client\n");
            exit(1);
        }

        if (strcmp(message, "get") == 0) {
            get_file(serverFd, clientAddr, clientLen);
        }
        if (strcmp(message, "put") == 0) {
            put_file(serverFd, clientAddr, clientLen);
        }
    }

    close(serverFd);
    exit(0);
}
