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


void handle_fault(char* errMsg, int recFd) {
    char* message;

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        printf("tcp-proxy: failed to allocate necessary memory\n");
        return;
    }

    printf("%s", errMsg);
    sprintf(message, "%s", "error");
    if (send(recFd, message, INPUT_MAX, 0) == -1) {
        printf("tcp-server: failed to transmit proxy error\n");
    }
}


void get_file(int clientFd, int serverFd) {
    char* message;
    FILE* fPtr;
    char* fName;
    long int fSize;
    long int rSize;
    int n;

    message = calloc(INPUT_MAX, sizeof(char));
    fName = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL || fName == NULL) {
        return;
    }

    sprintf(message, "%s", "get");
    if (send(serverFd, message, INPUT_MAX, 0) == -1) {
        handle_fault("tcp-proxy: failed to transmit the get command\n", clientFd);
        return;
    }
    if (recv(serverFd, message, INPUT_MAX, 0) == -1) {
        handle_fault("tcp-proxy: failed to receive get command response\n", clientFd);
        return;
    }
    if (strcmp(message, "ready") != 0) {
        handle_fault("tcp-proxy: received unexpected get command response\n", clientFd);
        return;
    }
    memset(message, 0, INPUT_MAX);

    sprintf(message, "%s", "ready");
    if (send(clientFd, message, INPUT_MAX, 0) == - 1) {
        handle_fault("tcp-proxy: failed to send get file name response\n", serverFd);
        return;
    }
    memset(message, 0, INPUT_MAX);

    if (recv(clientFd, fName, INPUT_MAX, 0) == -1) {
        handle_fault("tcp-proxy: failed to receive get file name\n", serverFd);
        return;
    }

    if (send(serverFd, fName, INPUT_MAX, 0) == -1) {
        handle_fault("tcp-proxy: failed to transmit the get file name\n", clientFd);
        return;
    }
    if (recv(serverFd, message, INPUT_MAX, 0) == -1) {
        handle_fault("tcp-proxy: failed to receive get file name response\n", clientFd);
        return;
    }
    if (strcmp(message, "ready") != 0) {
        handle_fault("tcp-proxy: received unexpected get file name response\n", clientFd);
        return;
    }
    memset(message, 0, INPUT_MAX);

    sprintf(message, "%s", "ready");
    if (send(clientFd, message, INPUT_MAX, 0) == -1) {
        handle_fault("tcp-proxy: failed to send get file size response\n", serverFd);
        return;
    }
    memset(message, 0, INPUT_MAX);

    if (recv(serverFd, &fSize, sizeof(fSize), 0) == -1) {
        handle_fault("x-client: failed to receive the file size\n", clientFd);
        return;
    }

    if (send(clientFd, &fSize, sizeof(fSize), 0) == -1) {
        handle_fault("tcp-proxy: failed to transmit the file size\n", serverFd);
        return;
    }
    if (recv(clientFd, message, INPUT_MAX, 0) == -1) {
        handle_fault("tcp-proxy: failed to receive get file size response\n", serverFd);
        return;
    }
    if (strcmp(message, "ready") != 0) {
        handle_fault("tcp-proxy: received unexpected get file size response\n", serverFd);
        return;
    }
    memset(message, 0, INPUT_MAX);

    printf("tcp-proxy: proxying the file...\n");
    while (fSize > 0) {
        rSize = recv(serverFd, message, INPUT_MAX, 0);
        if (rSize == -1) {
            handle_fault("tcp-proxy: failed to proxy the entire file\n", clientFd);
            return;
        }
        for (n = 0; n < )
        if (send(client, message, INPUT_MAX, 0) == -1) {
            handle_fault("tcp-proxy: failed to proxy the entire file\n", clientFd);
            return;
        }

        memset(message, 0, INPUT_MAX);
        fSize -= rSize;
    }
    printf("tcp-proxy: file successfully proxied\n");

    fclose(fPtr);
}


void put_file(int clientFd, int serverFd) {

}


int main(int argc, char* argv[]) {
    char cmd;
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
            rResult = recv(clientFd, message, INPUT_MAX, 0);
            if (rResult == 0) {
                break;
            }
            if (rResult == -1) {
                printf("tcp-proxy: failed to receive command from client\n");
                exit(1);
            }

            if (strcmp(message, "get") == 0) {
                get_file(clientFd, serverFd);
            }
            if (strcmp(message, "put") == 0) {
                put_file(clientFd, serverFd);
            }
        }
    }

    close(clientFd);
    close(serverFd);
    exit(0);
}