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


void get_file(int hostFd, struct sockaddr_storage* clientAddr, socklen_t clientLen) {
    char* message;
    char* fName;

    message = calloc(INPUT_MAX, sizeof(char));
    fName = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL || fName == NULL) {
        return;
    }

    sprintf(message, "%s", "ready");
    if (sendto(hostFd, message, INPUT_MAX, 0, (struct sockaddr*) clientAddr, clientLen) == - 1) {
        printf("udp-server: failed to send get file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    if (recvfrom(hostFd, message, INPUT_MAX, 0, (struct sockaddr*) clientAddr, &clientLen) == -1) {
        printf("udp-server: failed to receive get file name\n");
        return;
    }
    printf("got here\n");

    if (access(fName, F_OK)) {
        printf("udp-server: file %s does not exist\n", fName);
        sprintf(message, "%s", "error");
        if (sendto(hostFd, message, INPUT_MAX, 0, (struct sockaddr*) clientAddr, clientLen) == -1) {
            printf("udp-server: failed to send file does not exist error\n");
        }
        return;
    }
    memset(message, 0, INPUT_MAX);

    sprintf(message, "%s", "ready");
    if (sendto(hostFd, message, INPUT_MAX, 0, (struct sockaddr*) clientAddr, clientLen) == -1) {
        printf("udp-server: failed to send get file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    udp_file_transmit("udp-server", hostFd, fName, *clientAddr, clientLen);
}


void put_file(int hostFd, struct sockaddr_storage clientAddr, socklen_t clientLen)  {
    char* message;
    char* fName;

    message = calloc(INPUT_MAX, sizeof(char));
    fName = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL || fName == NULL) {
        printf("udp-server: failed to allocate necessary memory\n");
        return;
    }

    sprintf(message, "%s", "ready");
    if (sendto(hostFd, message, INPUT_MAX, 0, (struct sockaddr*) &clientAddr, clientLen) == - 1) {
        printf("udp-server: failed to send put file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    if (recvfrom(hostFd, fName, INPUT_MAX, 0, (struct sockaddr*) &clientAddr, &clientLen) == -1) {
        printf("udp-server: failed to receive put file name\n");
        return;
    }

    if (!access(fName, F_OK)) {
        printf("udp-server: file %s already exists\n", fName);
        sprintf(message, "%s", "error");
        if (sendto(hostFd, message, INPUT_MAX, 0, (struct sockaddr*) &clientAddr, clientLen) == -1) {
            printf("udp-server: failed to send file exists error\n");
        }
        return;
    }
    memset(message, 0, INPUT_MAX);

    sprintf(message, "%s", "ready");
    if (sendto(hostFd, message, INPUT_MAX, 0, (struct sockaddr*) &clientAddr, clientLen) == - 1) {
        printf("udp-server: failed to send put file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    udp_file_receive("udp-server", hostFd, fName, clientAddr, clientLen);
}


int main(int argc, char* argv[]) {
    char* hName;
    char* hPort;
    int hostFd;
    struct addrinfo hostInfo;
    struct sockaddr_storage clientAddr;
    socklen_t clientLen;
    char* message;
    int rSize;

    if (argc != 2) {
        printf("usage: ./udp-server <host port>\n");
        exit(1);
    }

    hPort = argv[1];
    if (!check_port(hPort)) {
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
    if (!udp_socket(&hostFd, &hostInfo, hName, hPort)) {
        printf("udp-server: failed to create udp socket for given host\n");
        exit(1);
    }
    if (bind(hostFd, hostInfo.ai_addr, hostInfo.ai_addrlen) == -1) {
        printf("udp-server: failed to bind udp socket for given host\n");
        exit(1);
    }

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        printf("udp-server: failed to allocate necessary memory\n");
        exit(1);
    }

    while (1) {
        rSize = recvfrom(hostFd, message, INPUT_MAX, 0, (struct sockaddr*) &clientAddr, &clientLen);
        if (rSize == -1) {
            printf("udp-server: failed to receive command from client\n");
            exit(1);
        }

        if (strcmp(message, "get") == 0) {
            get_file(hostFd, &clientAddr, clientLen);
        }
        if (strcmp(message, "put") == 0) {
            put_file(hostFd, clientAddr, clientLen);
        }
    }

    close(hostFd);
    exit(0);
}
