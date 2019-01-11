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


int tcp_client_get(char* prog, int serverFd, char* lFile, char* rFile) {
    char* message;

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        printf("%s: failed to allocate necessary memory\n", prog);
        return 0;
    }

    sprintf(message, "%s", "get");
    if (send(serverFd, message, INPUT_MAX, 0) == -1) {
        printf("%s: failed to transmit the get command\n", prog);
        return 0;
    }
    if (recv(serverFd, message, INPUT_MAX, 0) == -1) {
        printf("%s: failed to receive get command response\n", prog);
        return 0;
    }
    if (strcmp(message, "ready") != 0) {
        printf("%s: received unexpected get command response\n", prog);
        return 0;
    }
    memset(message, 0, INPUT_MAX);

    if (send(serverFd, rFile, INPUT_MAX, 0) == -1) {
        printf("%s: failed to transmit the get file name\n", prog);
        return 0;
    }
    if (recv(serverFd, message, INPUT_MAX, 0) == -1) {
        printf("%s: failed to receive get file name response\n", prog);
        return 0;
    }
    if (strcmp(message, "ready") != 0) {
        printf("%s: received unexpected get file name response\n", prog);
        return 0;
    }
    memset(message, 0, INPUT_MAX);

    return tcp_file_receive(prog, serverFd, lFile);
}


int tcp_client_put(char* prog, int serverFd, char* lFile, char* rFile) {
    char* message;
    FILE* fPtr;

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        printf("%s: failed to allocate necessary memory\n", prog);
        return 0;
    }

    fPtr = fopen(lFile, "r");
    if (fPtr == NULL) {
        printf("%s: failed to open file %s", lFile, prog);
        return 0;
    }

    sprintf(message, "%s", "put");
    if (send(serverFd, message, INPUT_MAX, 0) == -1) {
        printf("%s: failed to transmit the put command\n", prog);
        return 0;
    }
    if (recv(serverFd, message, INPUT_MAX, 0) == -1) {
        printf("%s: failed to receive put command response\n", prog);
        return 0;
    }
    if (strcmp(message, "ready") != 0) {
        printf("%s: received unexpected put command response\n", prog);
        return 0;
    }
    memset(message, 0, INPUT_MAX);

    if (send(serverFd, rFile, INPUT_MAX, 0) == -1) {
        printf("%s: failed to transmit the put file name\n", prog);
        return 0;
    }
    if (recv(serverFd, message, INPUT_MAX, 0) == -1) {
        printf("%s: failed to receive put file name response\n", prog);
        return 0;
    }
    if (strcmp(message, "ready") != 0) {
        printf("%s: received unexpected put file name response\n", prog);
        return 0;
    }
    memset(message, 0, INPUT_MAX);

    return tcp_file_transmit(prog, serverFd, lFile);
}


int tcp_server_get(char* prog, int clientFd, char* fName, int eCheck) {
    char* message;

    message = calloc(INPUT_MAX, sizeof(char));
    fName = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL || fName == NULL) {
        return 0;
    }

    sprintf(message, "%s", "ready");
    if (send(clientFd, message, INPUT_MAX, 0) == - 1) {
        printf("%s: failed to send get file name response\n", prog);
        return 0;
    }
    memset(message, 0, INPUT_MAX);

    if (recv(clientFd, fName, INPUT_MAX, 0) == -1) {
        printf("%s: failed to receive get file name\n", prog);
        return 0;
    }

    if (eCheck && access(fName, F_OK)) {
        printf("%s: file %s does not exist\n", fName, prog);
        sprintf(message, "%s", "error");
        if (send(clientFd, message, INPUT_MAX, 0) == -1) {
            printf("%s: failed to send file does not exist error\n", prog);
        }
        return 0;
    }
    memset(message, 0, INPUT_MAX);

    return tcp_file_transmit(prog, clientFd, fName);
}


int tcp_server_put(char* prog, int clientFd, char* fName, int eCheck) {
    char* message;

    message = calloc(INPUT_MAX, sizeof(char));
    fName = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL || fName == NULL) {
        printf("%s: failed to allocate necessary memory\n", prog);
        return 0;
    }

    sprintf(message, "%s", "ready");
    if (send(clientFd, message, INPUT_MAX, 0) == - 1) {
        printf("%s: failed to send put file name response\n", prog);
        return 0;
    }
    memset(message, 0, INPUT_MAX);

    if (recv(clientFd, fName, INPUT_MAX, 0) == -1) {
        printf("%s: failed to receive put file name\n", prog);
        return 0;
    }

    sprintf(message, "%s", "ready");
    if (send(clientFd, message, INPUT_MAX, 0) == -1) {
        printf("%s: failed to send put file size response\n", prog);
        return 0;
    }
    memset(message, 0, INPUT_MAX);

    if (eCheck && !access(fName, F_OK)) {
        printf("%s: file %s already exists\n", fName, prog);
        sprintf(message, "%s", "error");
        if (send(clientFd, message, INPUT_MAX, 0) == -1) {
            printf("%s: failed to send file exists error\n", prog);
        }
        return 0;
    }
    memset(message, 0, INPUT_MAX);

    return tcp_file_receive(prog, clientFd, fName);
}


int tcp_file_receive(char* prog, int serverFd, char* lFile) {
    char* message;
    FILE* fPtr;
    long int fSize;
    long int rSize;

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        printf("%s: failed to allocate necessary memory\n", prog);
        return 0;
    }

    fPtr = fopen(lFile, "a");
    if (fPtr == NULL) {
        printf("%s: failed to create file %s", lFile, prog);
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
