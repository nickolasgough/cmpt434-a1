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


#define STD_IN 0

#define PORT_MIN 30000
#define PORT_MAX 40000

#define INPUT_MAX 1000


int check_port(char* p) {
    if (atoi(p) < PORT_MIN) {
        return 0;
    }
    if (atoi(p) > PORT_MAX) {
        return 0;
    }

    return 1;
}


void get_file(int clientFd) {
    char* message;
    FILE* fPtr;
    char* fName;
    long int fSize;

    message = calloc(INPUT_MAX, sizeof(char));
    fName = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL || fName == NULL) {
        return;
    }

    sprintf(message, "%s", "ready");
    if (send(clientFd, message, INPUT_MAX, 0) == - 1) {
        printf("tcp-server: failed to send get file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    if (recv(clientFd, fName, INPUT_MAX, 0) == -1) {
        printf("tcp-server: failed to receive get file name\n");
        return;
    }

    if (access(fName, F_OK)) {
        printf("tcp-server: file %s does not exist\n", fName);
        sprintf(message, "%s", "error");
        if (send(clientFd, message, INPUT_MAX, 0) == -1) {
            printf("tcp-server: failed to send file does not exist error\n");
        }
        return;
    }
    memset(message, 0, INPUT_MAX);

    fPtr = fopen(fName, "r");
    if (fPtr == NULL) {
        printf("tcp-server: failed to open the file\n");
        sprintf(message, "%s", "error");
        if (send(clientFd, message, INPUT_MAX, 0) == -1) {
            printf("tcp-server: failed to send file open error\n");
        }
        return;
    }
    memset(message, 0, INPUT_MAX);

    sprintf(message, "%s", "ready");
    if (send(clientFd, message, INPUT_MAX, 0) == -1) {
        printf("tcp-server: failed to send get file size response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    fseek(fPtr, 0, SEEK_END);
    fSize = ftell(fPtr);
    fseek(fPtr, 0, SEEK_SET);

    if (send(clientFd, &fSize, sizeof(fSize), 0) == -1) {
        printf("tcp-server: failed to transmit the file size\n");
        return;
    }
    if (recv(clientFd, message, INPUT_MAX, 0) == -1) {
        printf("tcp-server: failed to receive get file size response\n");
        return;
    }
    if (strcmp(message, "ready") != 0) {
        printf("tcp-server: received unexpected get file size response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    printf("tcp-server: transmitting the file...\n");
    while (fread(message, sizeof(char), INPUT_MAX, fPtr) > 0) {
        if (send(clientFd, message, INPUT_MAX, 0) == -1) {
            printf("tcp-server: failed to get the whole file\n");
            return;
        }

        memset(message, 0, INPUT_MAX);
    }
    printf("tcp-server: file successfully transmitted\n");

    fclose(fPtr);
}


void put_file(int clientFd)  {
    char* message;
    FILE* fPtr;
    char* fName;
    long int fSize;
    long int rSize;

    message = calloc(INPUT_MAX, sizeof(char));
    fName = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL || fName == NULL) {
        return;
    }

    sprintf(message, "%s", "ready");
    if (send(clientFd, message, INPUT_MAX, 0) == - 1) {
        printf("tcp-server: failed to send put file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    if (recv(clientFd, fName, INPUT_MAX, 0) == -1) {
        printf("tcp-server: failed to receive put file name\n");
        return;
    }

    sprintf(message, "%s", "ready");
    if (send(clientFd, message, INPUT_MAX, 0) == -1) {
        printf("tcp-server: failed to send put file size response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    if (!access(fName, F_OK)) {
        printf("tcp-server: file %s already exists\n", fName);
        sprintf(message, "%s", "error");
        if (send(clientFd, message, INPUT_MAX, 0) == -1) {
            printf("tcp-server: failed to send file exists error\n");
        }
        return;
    }
    memset(message, 0, INPUT_MAX);

    if (recv(clientFd, &fSize, sizeof(fSize), 0) == -1) {
        printf("tcp-server: failed to receive put file size response\n");
        return;
    }

    sprintf(message, "%s", "ready");
    if (send(clientFd, message, INPUT_MAX, 0) == -1) {
        printf("tcp-server: failed to send put file contents response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    fPtr = fopen(fName, "a");
    if (fPtr == NULL) {
        printf("tcp-server: failed to create the file\n");
        sprintf(message, "%s", "error");
        if (send(clientFd, message, INPUT_MAX, 0) == -1) {
            printf("tcp-server: failed to send file creation error\n");
        }
        return;
    }
    memset(message, 0, INPUT_MAX);

    printf("tcp-server: receiving the file...\n");
    while (fSize > 0) {
        rSize = recv(clientFd, message, INPUT_MAX, 0);
        fwrite(message, sizeof(char), INPUT_MAX, fPtr);

        memset(message, 0, INPUT_MAX);
        fSize -= rSize;
    }
    printf("tcp-server: successfully received the file\n");

    fclose(fPtr);
}


int main(int argc, char* argv[]) {
    char* hName;
    char* port;
    int sockFd;
    struct addrinfo sockInfo;
    int clientFd;
    struct sockaddr clientAddr;
    socklen_t clientLen;
    char* message;
    int rResult;
    int qMax = 5;

    if (argc != 2) {
        printf("usage: ./tcp-server <port number>\n");
        exit(1);
    }

    port = argv[1];
    if (!check_port(port)) {
        printf("tcp-server: port number must be between 30000 and 40000\n");
        exit(1);
    }

    hName = calloc(INPUT_MAX, sizeof(char));
    if (hName == NULL) {
        printf("tcp-server: failed to allocate necessary memory\n");
        exit(1);
    }

    if (gethostname(hName, INPUT_MAX) == -1) {
        printf("tcp-server: failed to determine the name of the machine\n");
        exit(1);
    }
    if (!socket_tcp(&sockFd, &sockInfo, hName, port)) {
        printf("tcp-server: failed to create tcp socket for given host\n");
        exit(1);
    }

    if (bind(sockFd, sockInfo.ai_addr, sockInfo.ai_addrlen) == -1) {
        printf("tcp-server: failed to bind tcp socket for given host\n");
        exit(1);
    }
    if (listen(sockFd, qMax) == -1) {
        printf("tcp-server: failed to listen tcp socket for given host\n");
        exit(1);
    }

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        printf("tcp-server: failed to allocate necessary memory\n");
        exit(1);
    }

    while (1) {
        clientLen = sizeof(clientAddr);
        clientFd = accept(sockFd, &clientAddr, &clientLen);
        if (clientFd == -1) {
            printf("tcp-server: failed to accept incoming connection on socket\n");
            exit(1);
        }

        while (1) {
            rResult = recv(clientFd, message, INPUT_MAX, 0);
            if (rResult == 0) {
                break;
            }
            if (rResult == -1) {
                printf("tcp-server: failed to receive command from client\n");
                exit(1);
            }

            if (strcmp(message, "get") == 0) {
                get_file(clientFd);
            }
            if (strcmp(message, "put") == 0) {
                put_file(clientFd);
            }
        }
    }

    close(sockFd);
    exit(0);
}
