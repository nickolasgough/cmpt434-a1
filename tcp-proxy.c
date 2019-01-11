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


int proc_file(char* fDest, char* tDest) {
    long int fSize;
    long int fIndex;
    long int tIndex;
    char c;

    fSize = strlen(fDest);
    tDest = calloc(2 * fSize, sizeof(char));
    if (tDest == NULL) {
        return 0;
    }

    fIndex = 0;
    tIndex = 0;
    while (fIndex < fSize) {
        c = fDest[fIndex];
        tDest[tIndex] = c;
        fIndex += 1;
        tIndex += 1;

        if (c == 'c' || c == 'm' || c == 'p' || c == 't') {
            tDest[tIndex] = c;
            tIndex += 1;
        }
    }

    return 1;
}


void get_file(int clientFd, int serverFd) {
    char* message;
    char* fName = NULL;
    char* fDest = NULL;
    char* tDest = NULL;

    message = calloc(INPUT_MAX, sizeof(char));
    fName = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL || fName == NULL) {
        printf("tcp-proxy: failed to allocate necessary memory\n");
        return;
    }

    /* Initiate request with client */
    sprintf(message, "%s", "ready");
    if (send(clientFd, message, INPUT_MAX, 0) == - 1) {
        printf("tcp-proxy: failed to send get file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    if (recv(clientFd, fName, INPUT_MAX, 0) == -1) {
        printf("tcp-proxy: failed to receive get file name\n");
        return;
    }

    /* Initiate request with server */
    sprintf(message, "%s", "get");
    if (send(serverFd, message, INPUT_MAX, 0) == -1) {
        printf("x-client: failed to transmit the get command\n");
        return;
    }
    if (recv(serverFd, message, INPUT_MAX, 0) == -1) {
        printf("x-client: failed to receive get command response\n");
        return;
    }
    if (strcmp(message, "ready") != 0) {
        printf("x-client: received unexpected get command response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    if (send(serverFd, fName, INPUT_MAX, 0) == -1) {
        printf("x-client: failed to transmit the get file name\n");
        return;
    }
    if (recv(serverFd, message, INPUT_MAX, 0) == -1) {
        printf("x-client: failed to receive get file name response\n");
        return;
    }
    if (strcmp(message, "ready") != 0) {
        printf("x-client: received unexpected get file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    /* Receive file contents from server */
    if (!tcp_array_receive("tcp-proxy", serverFd, fDest)) {
        printf("tcp-proxy: failed to receive the file from the server\n");
        sprintf(message, "%s", "error");
        if (send(clientFd, message, INPUT_MAX, 0) == -1) {
            printf("tcp-proxy: failed to send proxy interruption error\n");
        }
        return;
    }

    /* Process the given file */
    if (!proc_file(fDest, tDest)) {
        printf("tcp-proxy: failed to process the given file\n");
        sprintf(message, "%s", "error");
        if (send(clientFd, message, INPUT_MAX, 0) == -1) {
            printf("tcp-proxy: failed to send proxy interruption error\n");
        }
        return;
    }

    /* Send file contents to client */
    sprintf(message, "%s", "ready");
    if (send(clientFd, message, INPUT_MAX, 0) == - 1) {
        printf("tcp-proxy: failed to send get file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    if (!tcp_array_transmit("tcp-proxy", clientFd, tDest)) {
        printf("tcp-proxy: failed tp transmit the file to the client\n");
    }
}


void put_file(int clientFd, int serverFd) {
    char* message;
    char* fName = NULL;
    char* fDest = NULL;
    char* tDest = NULL;

    message = calloc(INPUT_MAX, sizeof(char));
    fName = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL || fName == NULL) {
        printf("tcp-proxy: failed to allocate necessary memory\n");
        return;
    }

    /* Initiate request with client */
    message = calloc(INPUT_MAX, sizeof(char));
    fName = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL || fName == NULL) {
        printf("tcp-proxy: failed to allocate necessary memory\n");
        return;
    }

    sprintf(message, "%s", "ready");
    if (send(clientFd, message, INPUT_MAX, 0) == - 1) {
        printf("tcp-proxy: failed to send put file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    if (recv(clientFd, fName, INPUT_MAX, 0) == -1) {
        printf("tcp-proxy: failed to receive put file name\n");
        return;
    }

    sprintf(message, "%s", "ready");
    if (send(clientFd, message, INPUT_MAX, 0) == - 1) {
        printf("tcp-proxy: failed to send put file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    /* Receive file contents from client */
    if (!tcp_array_receive("tcp-proxy", clientFd, fDest)) {
        printf("tcp-proxy: failed to receive the file from the client\n");
        return;
    }

    /* Process given file contents */
    if (!proc_file(fDest, tDest)) {
        printf("tcp-proxy: failed to process the given file\n");
        sprintf(message, "%s", "error");
        if (send(clientFd, message, INPUT_MAX, 0) == -1) {
            printf("tcp-proxy: failed to send proxy interruption error\n");
        }
        return;
    }

    if (!tcp_array_transmit("tcp-proxy", serverFd, tDest)) {
        printf("tcp-proxy: failed tp transmit the file to the client\n");
    }
}


int main(int argc, char* argv[]) {
    char* cmd;
    char* lFile;
    char* rFile;
    char* hName;
    char* hPort;
    char* sName;
    char* sPort;
    int serverFd;
    struct addrinfo serverInfo;
    int hostFd;
    struct addrinfo hostInfo;
    int clientFd;
    struct sockaddr clientAddr;
    socklen_t clientLen;
    int rResult;
    int qMax = 1;

    if (argc != 4) {
        printf("usage: ./tcp-proxy <host port> <server name> <server port>\n");
        exit(1);
    }

    hPort = argv[1];
    sName = argv[2];
    sPort = argv[3];
    if (!check_port(hPort) || !check_port(sPort)) {
        printf("tcp-proxy: port number must be between 30000 and 40000\n");
        exit(1);
    }

    hName = calloc(INPUT_MAX, sizeof(char));
    if (hName == NULL) {
        printf("tcp-proxy: failed to allocate necessary memory\n");
        exit(1);
    }

    if (gethostname(hName, INPUT_MAX) == -1) {
        printf("tcp-proxy: failed to determine the name of the machine\n");
        exit(1);
    }
    if (!socket_tcp(&hostFd, &hostInfo, hName, hPort)) {
        printf("tcp-proxy: failed to create tcp socket for given host\n");
        exit(1);
    }
    if (bind(hostFd, hostInfo.ai_addr, hostInfo.ai_addrlen) == -1) {
        printf("tcp-proxy: failed to bind tcp socket for given host\n");
        exit(1);
    }

    if (!socket_tcp(&serverFd, &serverInfo, sName, sPort)) {
        printf("tcp-proxy: failed to create tcp socket for given host\n");
        exit(1);
    }
    if (connect(serverFd, serverInfo.ai_addr, serverInfo.ai_addrlen) == -1) {
        printf("tcp-proxy: failed to connect tcp socket for given host\n");
        exit(1);
    }

    if (listen(hostFd, qMax) == -1) {
        printf("tcp-proxy: failed to listen tcp socket for given host\n");
        exit(1);
    }

    cmd = calloc(INPUT_MAX, sizeof(char));
    lFile = calloc(INPUT_MAX, sizeof(char));
    rFile = calloc(INPUT_MAX, sizeof(char));
    if (cmd == NULL || rFile == NULL || lFile == NULL) {
        printf("tcp-proxy: failed to allocate necessary memory\n");
        exit(1);
    }

    while (1) {
        clientLen = sizeof(clientAddr);
        clientFd = accept(hostFd, &clientAddr, &clientLen);
        if (clientFd == -1) {
            printf("tcp-proxy: failed to accept incoming connection on socket\n");
            exit(1);
        }

        while (1) {
            rResult = recv(clientFd, cmd, INPUT_MAX, 0);
            if (rResult == 0) {
                break;
            }
            if (rResult == -1) {
                printf("tcp-proxy: failed to receive command from client\n");
                exit(1);
            }

            if (strcmp(cmd, "get") == 0) {
                get_file(clientFd, serverFd);
            }
            if (strcmp(cmd, "put") == 0) {
                put_file(clientFd, serverFd);
            }
        }
    }

    close(hostFd);
    close(clientFd);
    close(serverFd);
    exit(0);
}