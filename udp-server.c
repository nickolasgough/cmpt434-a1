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


#define MYPORT "30002"


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


int get_socket() {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    return sockfd;
}


int main(int argc, char* argv[]) {
    int sockfd;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[INPUT_MAX];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
    char* message;

    sockfd = get_socket();

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        printf("udp-server: failed to allocate necessary memory\n");
        exit(1);
    }

    addr_len = sizeof(their_addr);
    if (recvfrom(sockfd, message, INPUT_MAX, 0, (struct sockaddr*) &their_addr, &addr_len) == -1) {
        printf("udp-server: failed to receive from client\n");
        exit(1);
    }
    printf("%s\n", message);
    handle(sockfd, &their_addr, addr_len);

    close(sockfd);
    return 0;
}
