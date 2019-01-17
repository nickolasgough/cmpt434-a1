/* Nickolas Gough, nvg081, 11181823 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>

#include "x-sockets.h"
#include "x-common.h"


void handle(int hostFd, struct sockaddr_storage* replyhere, socklen_t storageLen) {
    char* message;

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        printf("udp-server: failed to allocate necessary memory\n");
        exit(1);
    }

    if (sendto(hostFd, "hello", INPUT_MAX, 0, (struct sockaddr*) replyhere, sizeof(struct sockaddr_storage)) == -1) {
        printf("udp-server: failed to reply to client\n");
        printf("Error: %d - %s\n", errno, strerror(errno));
        exit(1);
    }
    if (recvfrom(hostFd, message, INPUT_MAX, 0, (struct sockaddr*) replyhere, &storageLen) == -1) {
        printf("udp-server: failed to receive from client\n");
        exit(1);
    }
    printf("%s\n", message);
    if (sendto(hostFd, "hello", INPUT_MAX, 0, (struct sockaddr*) replyhere, sizeof(struct sockaddr_storage)) == -1) {
        printf("udp-server: failed to reply to client\n");
        printf("Error: %d - %s\n", errno, strerror(errno));
        exit(1);
    }
}


int main(int argc, char* argv[]) {
    char* hName;
    char* hPort;
    int hostFd;
    struct addrinfo hostInfo;
    struct sockaddr_storage clientAddr;
    socklen_t clientLen;
    char* message;

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

    if (!udp_socket(&hostFd, &hostInfo, NULL, hPort)) {
        printf("udp-server: failed to create udp socket for given host\n");
        exit(1);
    }

    if (bind(hostFd, hostInfo.ai_addr, hostInfo.ai_addrlen) == -1) {
        printf("udp-server: failed to bind udp socket for given host\n");
        exit(1);
    }
    freeaddrinfo(&hostInfo);

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        printf("udp-server: failed to allocate necessary memory\n");
        exit(1);
    }

    clientLen = sizeof(clientAddr);
    if (recvfrom(hostFd, message, INPUT_MAX-1, 0, (struct sockaddr*) &clientAddr, &clientLen) == -1) {
        printf("udp-server: failed to receive from client\n");
        exit(1);
    }
    printf("%s\n", message);
    handle(hostFd, &clientAddr, clientLen);

    close(hostFd);
    return 0;
}
