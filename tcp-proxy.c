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


void proc_file(char* file, char* temp) {
    FILE* fPtr;
    FILE* tPtr;
    char c;

    fPtr = fopen(file, "r");
    tPtr = fopen(temp, "a");
    if (fPtr == NULL || tPtr == NULL) {
        printf("tcp-proxy: failed to process file\n");
        return;
    }

    while (!feof(fPtr)) {
        c = fgetc(fPtr);

        fputc(c, tPtr);
        if (c == 'c' || c == 'm' || c == 'p' || c == 't') {
            fputc(c, tPtr);
        }
    }

    fclose(fPtr);
    fclose(tPtr);
}


void get_file(int clientFd, int serverFd) {
    char* message;
    char* fName;

    message = calloc(INPUT_MAX, sizeof(char));
    fName = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL || fName == NULL) {
        printf("tcp-proxy: failed to allocate necessary memory\n");
        return;
    }

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

    if (!tcp_client_get("tcp-proxy", serverFd, TEMP0, fName)) {
        printf("tcp-proxy: failed to receive the file from the server\n");
        return;
    }

    proc_file(TEMP0, TEMP1);

    sprintf(message, "%s", "ready");
    if (send(clientFd, message, INPUT_MAX, 0) == -1) {
        printf("tcp-proxy: failed to send file size response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    if (!tcp_file_transmit("tcp-proxy", clientFd, TEMP1)) {
        printf("tcp-proxy: failed tp transmit the file to the client\n");
    }

    remove(TEMP0);
    remove(TEMP1);
}


void put_file(int clientFd, int serverFd) {
    char* message;
    char* fName;

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
    if (send(clientFd, message, INPUT_MAX, 0) == -1) {
        printf("tcp-proxy: failed to send put file size response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    if (!tcp_file_receive("tcp-proxy", clientFd, TEMP0)) {
        printf("tcp-proxy: failed to receive the file from the client\n");
        return;
    }

    proc_file(TEMP0, TEMP1);

    if (!tcp_client_put("tcp-proxy", serverFd, TEMP1, fName)) {
        printf("tcp-proxy: failed to transmit the file to the server\n");
    }

    remove(TEMP0);
    remove(TEMP1);
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
