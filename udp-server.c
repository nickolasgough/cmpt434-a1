/* Nickolas Gough, nvg081, 11181823 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>

#include "x-sockets.h"
#include "x-common.h"


void get_file(int clientFd, struct sockaddr clientAddr, socklen_t clientLen) {
    char* message;
    char* fName;

    message = calloc(INPUT_MAX, sizeof(char));
    fName = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL || fName == NULL) {
        return;
    }

    printf("Sending to proxy...\n");
    sprintf(message, "%s", "ready");
    if (sendto(clientFd, message, INPUT_MAX, 0, &clientAddr, clientLen) == - 1) {
        printf("udp-server: failed to send get file name response\n");
        printf("Error: %d - %s", errno, strerror(errno));
        return;
    }
    memset(message, 0, INPUT_MAX);

    printf("Receiving from proxy...\n");
    if (recvfrom(clientFd, fName, INPUT_MAX, 0, &clientAddr, &clientLen) == -1) {
        printf("udp-server: failed to receive get file name\n");
        return;
    }

    if (access(fName, F_OK)) {
        printf("udp-server: file %s does not exist\n", fName);
        sprintf(message, "%s", "error");
        if (sendto(clientFd, message, INPUT_MAX, 0, &clientAddr, clientLen) == -1) {
            printf("udp-server: failed to send file does not exist error\n");
        }
        return;
    }
    memset(message, 0, INPUT_MAX);

    sprintf(message, "%s", "ready");
    if (sendto(clientFd, message, INPUT_MAX, 0, &clientAddr, clientLen) == -1) {
        printf("udp-server: failed to send get file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    udp_file_transmit("udp-server", clientFd, fName, clientAddr, clientLen);
}


void put_file(int clientFd, struct sockaddr clientAddr, socklen_t clientLen)  {
    char* message;
    char* fName;

    message = calloc(INPUT_MAX, sizeof(char));
    fName = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL || fName == NULL) {
        printf("udp-server: failed to allocate necessary memory\n");
        return;
    }

    sprintf(message, "%s", "ready");
    if (sendto(clientFd, message, INPUT_MAX, 0, &clientAddr, clientLen) == - 1) {
        printf("udp-server: failed to send put file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    if (recvfrom(clientFd, fName, INPUT_MAX, 0, &clientAddr, &clientLen) == -1) {
        printf("udp-server: failed to receive put file name\n");
        return;
    }

    if (!access(fName, F_OK)) {
        printf("udp-server: file %s already exists\n", fName);
        sprintf(message, "%s", "error");
        if (sendto(clientFd, message, INPUT_MAX, 0, &clientAddr, clientLen) == -1) {
            printf("udp-server: failed to send file exists error\n");
        }
        return;
    }
    memset(message, 0, INPUT_MAX);

    sprintf(message, "%s", "ready");
    if (sendto(clientFd, message, INPUT_MAX, 0, &clientAddr, clientLen) == - 1) {
        printf("udp-server: failed to send put file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    udp_file_receive("udp-server", clientFd, fName, clientAddr, clientLen);
}


int main(int argc, char* argv[]) {
    char* hName;
    char* hPort;
    char* sName;
    char* sPort;
    int serverFd;
    struct addrinfo serverInfo;
    int clientFd;
    struct addrinfo clientInfo;
    struct sockaddr clientAddr;
    socklen_t clientLen;
    char* message;
    int rSize;

    if (argc != 4) {
        printf("usage: ./udp-server <host port> <client name> <client port>\n");
        exit(1);
    }

    hPort = argv[1];
    sName = argv[2];
    sPort = argv[3];
    if (!check_port(hPort) || !check_port(sPort)) {
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
    if (!udp_socket(&serverFd, &serverInfo, hName, hPort)) {
        printf("udp-server: failed to create udp socket for given host\n");
        exit(1);
    }

    if (bind(serverFd, serverInfo.ai_addr, serverInfo.ai_addrlen) == -1) {
        printf("udp-server: failed to bind udp socket for given host\n");
        exit(1);
    }

    if (!udp_socket(&clientFd, &clientInfo, sName, sPort)) {
        printf("udp-server: failed to create udp socket for given client\n");
        exit(1);
    }

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        printf("udp-server: failed to allocate necessary memory\n");
        exit(1);
    }

    while (1) {
        memset(&clientAddr, 0, sizeof(clientAddr));

        rSize = recvfrom(serverFd, message, INPUT_MAX, 0, &clientAddr, &clientLen);
        if (rSize == -1) {
            printf("udp-server: failed to receive command from client\n");
            exit(1);
        }

        if (strcmp(message, "get") == 0) {
            get_file(clientFd, clientAddr, clientLen);
        }
        if (strcmp(message, "put") == 0) {
            put_file(clientFd, clientAddr, clientLen);
        }
    }

    close(serverFd);
    exit(0);
}
