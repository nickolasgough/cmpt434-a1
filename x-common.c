/* Nickolas Gough, nvg081, 11181823 */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#include "x-common.h"


int check_port(char* port) {
    if (atoi(port) < PORT_MIN) {
        return 0;
    }
    if (atoi(port) > PORT_MAX) {
        return 0;
    }

    return 1;
}


int tcp_file_receive(char* prog, int serverFd, char* fName) {
    char* message;
    FILE* fPtr;
    long int fSize;
    long int rSize;

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        printf("%s: failed to allocate necessary memory\n", prog);
        return 0;
    }

    fPtr = fopen(fName, "a");
    if (fPtr == NULL) {
        printf("%s: failed to create file %s", fName, prog);
        return 0;
    }

    if (recv(serverFd, &fSize, sizeof(fSize), 0) == -1) {
        printf("%s: failed to receive file size\n", prog);
        return 0;
    }

    sprintf(message, "%s", "ready");
    if (send(serverFd, message, INPUT_MAX, 0) == -1) {
        printf("%s: failed to send file size response\n", prog);
        return 0;
    }
    memset(message, 0, INPUT_MAX);

    printf("%s: receiving the file...\n", prog);
    while (fSize > 0) {
        rSize = recv(serverFd, message, INPUT_MAX, 0);
        if (rSize == -1) {
            printf("%s: failed to receive the whole file\n", prog);
            return 0;
        }
        fwrite(message, sizeof(char), INPUT_MAX, fPtr);

        memset(message, 0, INPUT_MAX);
        fSize -= rSize;
    }
    printf("%s: file successfully received\n", prog);

    fclose(fPtr);
    return 1;
}


int tcp_file_transmit(char* prog, int clientFd, char* fName) {
    char* message;
    FILE* fPtr;
    long int fSize;

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        printf("%s: failed to allocate necessary memory\n", prog);
        return 0;
    }

    fPtr = fopen(fName, "r");
    if (fPtr == NULL) {
        printf("%s: failed to open the file\n", prog);
        sprintf(message, "%s", "error");
        if (send(clientFd, message, INPUT_MAX, 0) == -1) {
            printf("%s: failed to send file open error\n", prog);
        }
        return 0;
    }
    memset(message, 0, INPUT_MAX);

    fseek(fPtr, 0, SEEK_END);
    fSize = ftell(fPtr);
    fseek(fPtr, 0, SEEK_SET);

    if (send(clientFd, &fSize, sizeof(fSize), 0) == -1) {
        printf("%s: failed to send file size\n", prog);
        return 0;
    }
    if (recv(clientFd, message, INPUT_MAX, 0) == -1) {
        printf("%s: failed to receive file size response\n", prog);
        return 0;
    }
    if (strcmp(message, "ready") != 0) {
        printf("%s: received unexpected file size response\n", prog);
        return 0;
    }
    memset(message, 0, INPUT_MAX);

    printf("%s: transmitting the file...\n", prog);
    while (fread(message, sizeof(char), INPUT_MAX, fPtr) > 0) {
        if (send(clientFd, message, INPUT_MAX, 0) == -1) {
            printf("%s: failed to transmit the whole file\n", prog);
            return 0;
        }

        memset(message, 0, INPUT_MAX);
    }
    printf("%s: file successfully transmitted\n", prog);

    fclose(fPtr);
    return 1;
}


char* tcp_array_receive(char* prog, int serverFd) {
    char* message;
    char* fDest;
    long int fSize;
    long int rSize;
    long int cPos;

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        printf("%s: failed to allocate necessary memory\n", prog);
        return NULL;
    }

    if (recv(serverFd, &fSize, sizeof(fSize), 0) == -1) {
        printf("%s: failed to receive file size\n", prog);
        return NULL;
    }

    fDest = calloc(fSize, sizeof(char));
    if (fDest == NULL) {
        printf("%s: failed to allocate necessary memory\n", prog);
        return NULL;
    }

    sprintf(message, "%s", "ready");
    if (send(serverFd, message, INPUT_MAX, 0) == -1) {
        printf("%s: failed to send file size response\n", prog);
        return NULL;
    }
    memset(message, 0, INPUT_MAX);

    printf("%s: receiving the file...\n", prog);
    cPos = 0;
    while (fSize > 0) {
        rSize = recv(serverFd, &fDest[cPos], fSize, 0);
        if (rSize == -1) {
            printf("%s: failed to receive the whole file\n", prog);
            return NULL;
        }

        fSize -= rSize;
        cPos += rSize;
    }
    printf("%s: file successfully received\n", prog);

    return fDest;
}


int tcp_array_transmit(char* prog, int clientFd, char* fDest) {
    char* message;
    long int fSize;
    long int sSize;
    long int cPos;

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        printf("%s: failed to allocate necessary memory\n", prog);
        return 0;
    }

    fSize = strlen(fDest);

    if (send(clientFd, &fSize, sizeof(fSize), 0) == -1) {
        printf("%s: failed to send file size\n", prog);
        return 0;
    }
    if (recv(clientFd, message, INPUT_MAX, 0) == -1) {
        printf("%s: failed to receive file size response\n", prog);
        return 0;
    }
    if (strcmp(message, "ready") != 0) {
        printf("%s: received unexpected file size response\n", prog);
        return 0;
    }
    memset(message, 0, INPUT_MAX);

    printf("%s: transmitting the file...\n", prog);
    cPos = 0;
    while (fSize > 0) {
        sSize = send(clientFd, &fDest[cPos], INPUT_MAX, 0);
        if (sSize == -1) {
            printf("%s: failed to transmit the whole file\n", prog);
            return 0;
        }

        fSize -= sSize;
        cPos += sSize;
    }
    printf("%s: file successfully transmitted\n", prog);

    return 1;
}
