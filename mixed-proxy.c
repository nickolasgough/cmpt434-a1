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
#include "x-common.h"


#define TEMP0 "temp0.txt"
#define TEMP1 "temp1.txt"


void get_file(int hostFd, int clientFd, int serverFd, struct sockaddr serverAddr, socklen_t serverLen) {
    char* message;
    char* fName = NULL;
    char* fDest = NULL;
    char* tDest = NULL;

    message = calloc(INPUT_MAX, sizeof(char));
    fName = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL || fName == NULL) {
        printf("mixed-proxy: failed to allocate necessary memory\n");
        return;
    }

    /* Initiate request with client */
    sprintf(message, "%s", "ready");
    if (send(clientFd, message, INPUT_MAX, 0) == -1) {
        printf("mixed-proxy: failed to send get file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    if (recv(clientFd, fName, INPUT_MAX, 0) == -1) {
        printf("mixed-proxy: failed to receive get file name\n");
        return;
    }

    sprintf(message, "%s", "ready");
    if (send(clientFd, message, INPUT_MAX, 0) == -1) {
        printf("mixed-proxy: failed to send get file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    /* Initiate request with server */
    sprintf(message, "%s", "get");
    if (sendto(serverFd, message, INPUT_MAX, 0, &serverAddr, serverLen) == -1) {
        printf("mixed-proxy: failed to transmit the get command\n");
        return;
    }
    if (recvfrom(serverFd, message, INPUT_MAX, 0, NULL, NULL) == -1) {
        printf("mixed-proxy: failed to receive get command response\n");
        return;
    }
    if (strcmp(message, "ready") != 0) {
        printf("mixed-proxy: received unexpected get command response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    if (sendto(serverFd, fName, INPUT_MAX, 0, &serverAddr, serverLen) == -1) {
        printf("mixed-proxy: failed to transmit the get file name\n");
        return;
    }
    if (recvfrom(hostFd, message, INPUT_MAX, 0, NULL, NULL) == -1) {
        printf("mixed-proxy: failed to receive get file name response\n");
        return;
    }
    if (strcmp(message, "ready") != 0) {
        printf("mixed-proxy: received unexpected get file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    /* Receive file contents from server */
    fDest = udp_array_receive("mixed-proxy", hostFd, serverFd, serverAddr, serverLen);
    if (fDest == NULL) {
        printf("mixed-proxy: failed to receive the file from the server\n");
        sprintf(message, "%s", "error");
        if (send(clientFd, message, INPUT_MAX, 0) == -1) {
            printf("mixed-proxy: failed to send proxy interruption error\n");
        }
        return;
    }

    /* Process the given file */
    tDest = proc_file(fDest);
    if (tDest == NULL) {
        printf("mixed-proxy: failed to process the given file\n");
        sprintf(message, "%s", "error");
        if (send(clientFd, message, INPUT_MAX, 0) == -1) {
            printf("mixed-proxy: failed to send proxy interruption error\n");
        }
        return;
    }

    /* Send file contents to client */
    if (!tcp_array_transmit("mixed-proxy", clientFd, tDest)) {
        printf("mixed-proxy: failed tp transmit the file to the client\n");
    }
}


void put_file(int hostFd, int clientFd, int serverFd, struct sockaddr serverAddr, socklen_t serverLen) {
    char* message;
    char* fName = NULL;
    char* fDest = NULL;
    char* tDest = NULL;

    message = calloc(INPUT_MAX, sizeof(char));
    fName = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL || fName == NULL) {
        printf("mixed-proxy: failed to allocate necessary memory\n");
        return;
    }

    /* Initiate request with client */
    message = calloc(INPUT_MAX, sizeof(char));
    fName = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL || fName == NULL) {
        printf("mixed-proxy: failed to allocate necessary memory\n");
        return;
    }

    sprintf(message, "%s", "ready");
    if (send(clientFd, message, INPUT_MAX, 0) == -1) {
        printf("mixed-proxy: failed to send put file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    if (recv(clientFd, fName, INPUT_MAX, 0) == -1) {
        printf("mixed-proxy: failed to receive put file name\n");
        return;
    }

    sprintf(message, "%s", "ready");
    if (send(clientFd, message, INPUT_MAX, 0) == -1) {
        printf("mixed-proxy: failed to send put file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    /* Receive file contents from client */
    fDest = tcp_array_receive("mixed-proxy", clientFd);
    if (fDest == NULL) {
        printf("mixed-proxy: failed to receive the file from the client\n");
        return;
    }

    /* Process given file contents */
    tDest = proc_file(fDest);
    if (tDest == NULL) {
        printf("mixed-proxy: failed to process the given file\n");
        return;
    }

    /* Initiate the request with the server */
    sprintf(message, "%s", "put");
    if (sendto(serverFd, message, INPUT_MAX, 0, &serverAddr, serverLen) == -1) {
        printf("mixed-proxy: failed to transmit the put command\n");
        return;
    }
    if (recvfrom(hostFd, message, INPUT_MAX, 0, NULL, NULL) == -1) {
        printf("mixed-proxy: failed to receive put command response\n");
        return;
    }
    if (strcmp(message, "ready") != 0) {
        printf("mixed-proxy: received unexpected put command response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    if (sendto(serverFd, fName, INPUT_MAX, 0, &serverAddr, serverLen) == -1) {
        printf("mixed-proxy: failed to transmit the put file name\n");
        return;
    }
    if (recvfrom(hostFd, message, INPUT_MAX, 0, NULL, NULL) == -1) {
        printf("mixed-proxy: failed to receive put file name response\n");
        return;
    }
    if (strcmp(message, "ready") != 0) {
        printf("mixed-proxy: received unexpected put file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    /* Send file contents to server */
    if (!udp_array_transmit("mixed-proxy", hostFd, serverFd, tDest, serverAddr, serverLen)) {
        printf("mixed-proxy: failed tp transmit the file to the client\n");
    }
}


int main(int argc, char* argv[]) {
    char* cmd;
    char* hName;
    char* hPort;
    char* sName;
    char* sPort;
    int hostTcpFd;
    struct addrinfo hostTcpInfo;
    int hostUdpFd;
    struct addrinfo hostUdpInfo;
    int serverFd;
    struct addrinfo serverInfo;
    int clientFd;
    struct sockaddr clientAddr;
    socklen_t clientLen;
    int rResult;
    int qMax = 1;

    if (argc != 4) {
        printf("usage: ./mixed-proxy <host port> <server name> <server port>\n");
        exit(1);
    }

    hPort = argv[1];
    sName = argv[2];
    sPort = argv[3];
    if (!check_port(hPort) || !check_port(sPort)) {
        printf("mixed-proxy: port number must be between 30000 and 40000\n");
        exit(1);
    }

    hName = calloc(INPUT_MAX, sizeof(char));
    if (hName == NULL) {
        printf("mixed-proxy: failed to allocate necessary memory\n");
        exit(1);
    }

    if (gethostname(hName, INPUT_MAX) == -1) {
        printf("mixed-proxy: failed to determine the name of the machine\n");
        exit(1);
    }
    if (!tcp_socket(&hostTcpFd, &hostTcpInfo, hName, hPort)) {
        printf("mixed-proxy: failed to create tcp socket for given host\n");
        exit(1);
    }
    if (bind(hostTcpFd, hostTcpInfo.ai_addr, hostTcpInfo.ai_addrlen) == -1) {
        printf("mixed-proxy: failed to bind tcp socket for given host\n");
        exit(1);
    }

    if (!udp_socket(&hostUdpFd, &hostUdpInfo, hName, hPort)) {
        printf("mixed-proxy: failed to create udp socket for given host\n");
        exit(1);
    }
    if (bind(hostUdpFd, hostUdpInfo.ai_addr, hostUdpInfo.ai_addrlen) == -1) {
        printf("mixed-proxy: failed to bind udp socket for given host\n");
        exit(1);
    }

    if (listen(hostTcpFd, qMax) == -1) {
        printf("mixed-proxy: failed to listen tcp socket for given host\n");
        exit(1);
    }
    if (!udp_socket(&serverFd, &serverInfo, sName, sPort)) {
        printf("mixed-proxy: failed to create udp socket for given server\n");
        exit(1);
    }

    cmd = calloc(INPUT_MAX, sizeof(char));
    if (cmd == NULL) {
        printf("mixed-proxy: failed to allocate necessary memory\n");
        exit(1);
    }

    while (1) {
        clientLen = sizeof(clientAddr);
        clientFd = accept(hostTcpFd, &clientAddr, &clientLen);
        if (clientFd == -1) {
            printf("mixed-proxy: failed to accept incoming connection on socket\n");
            exit(1);
        }

        while (1) {
            rResult = recv(clientFd, cmd, INPUT_MAX, 0);
            if (rResult == 0) {
                break;
            }
            if (rResult == -1) {
                printf("mixed-proxy: failed to receive command from client\n");
                exit(1);
            }

            if (strcmp(cmd, "get") == 0) {
                get_file(hostUdpFd, clientFd, serverFd, *serverInfo.ai_addr, serverInfo.ai_addrlen);
            }
            if (strcmp(cmd, "put") == 0) {
                put_file(hostUdpFd, clientFd, serverFd, *serverInfo.ai_addr, serverInfo.ai_addrlen);
            }
        }
    }

    close(hostTcpFd);
    close(clientFd);
    close(serverFd);
    exit(0);
}
