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


char* proc_file(char* fDest) {
    char* tDest;
    long int fSize;
    long int fIndex;
    long int tIndex;
    char c;

    fSize = strlen(fDest);
    tDest = calloc(2 * fSize, sizeof(char));
    if (tDest == NULL) {
        return NULL;
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

    return tDest;
}


int tcp_file_receive(char* prog, int recvFd, char* fName) {
    char* message;
    FILE* fPtr;
    long int fSize;
    long int rSize;
    long int rAmount;

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

    if (recv(recvFd, &fSize, sizeof(fSize), 0) == -1) {
        printf("%s: failed to receive file size\n", prog);
        return 0;
    }

    sprintf(message, "%s", "ready");
    if (send(recvFd, message, INPUT_MAX, 0) == -1) {
        printf("%s: failed to send file size response\n", prog);
        return 0;
    }
    memset(message, 0, INPUT_MAX);

    printf("%s: receiving the file...\n", prog);
    while (fSize > 0) {
        rAmount = fSize > INPUT_MAX ? INPUT_MAX : fSize;
        rSize = recv(recvFd, message, rAmount, 0);
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


int udp_file_receive(char* prog, int recvFd, char* fName, struct sockaddr_storage recvAddr, socklen_t recvLen) {
    char* message;
    FILE* fPtr;
    long int fSize;
    long int rSize;
    long int rAmount;

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

    if (recvfrom(recvFd, &fSize, sizeof(fSize), 0, (struct sockaddr*) &recvAddr, &recvLen) == -1) {
        printf("%s: failed to receive file size\n", prog);
        return 0;
    }

    sprintf(message, "%s", "ready");
    if (sendto(recvFd, message, INPUT_MAX, 0, (struct sockaddr*) &recvAddr, recvLen) == -1) {
        printf("%s: failed to send file size response\n", prog);
        return 0;
    }
    memset(message, 0, INPUT_MAX);

    printf("%s: receiving the file...\n", prog);
    while (fSize > 0) {
        rAmount = fSize > INPUT_MAX ? INPUT_MAX : fSize;
        rSize = recvfrom(recvFd, message, INPUT_MAX, 0, (struct sockaddr*) &recvAddr, &recvLen);
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


int tcp_file_transmit(char* prog, int transFd, char* fName) {
    char* message;
    FILE* fPtr;
    long int fSize;
    long int sSize;
    long int tAmount;

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        printf("%s: failed to allocate necessary memory\n", prog);
        return 0;
    }

    fPtr = fopen(fName, "r");
    if (fPtr == NULL) {
        printf("%s: failed to open the file\n", prog);
        sprintf(message, "%s", "error");
        if (send(transFd, message, INPUT_MAX, 0) == -1) {
            printf("%s: failed to send file open error\n", prog);
        }
        return 0;
    }
    memset(message, 0, INPUT_MAX);

    fseek(fPtr, 0, SEEK_END);
    fSize = ftell(fPtr);
    fseek(fPtr, 0, SEEK_SET);

    if (send(transFd, &fSize, sizeof(fSize), 0) == -1) {
        printf("%s: failed to send file size\n", prog);
        return 0;
    }
    if (recv(transFd, message, INPUT_MAX, 0) == -1) {
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
        tAmount = fSize > INPUT_MAX ? INPUT_MAX : fSize;
        sSize = send(transFd, message, tAmount, 0);
        if (sSize == -1) {
            printf("%s: failed to transmit the whole file\n", prog);
            return 0;
        }

        memset(message, 0, INPUT_MAX);
        fSize -= sSize;
    }
    printf("%s: file successfully transmitted\n", prog);

    fclose(fPtr);
    return 1;
}


int udp_file_transmit(char* prog, int transFd, char* fName, struct sockaddr_storage transAddr, socklen_t transLen) {
    char* message;
    FILE* fPtr;
    long int fSize;
    long int sSize;
    long int tAmount;

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        printf("%s: failed to allocate necessary memory\n", prog);
        return 0;
    }

    fPtr = fopen(fName, "r");
    if (fPtr == NULL) {
        printf("%s: failed to open the file\n", prog);
        sprintf(message, "%s", "error");
        if (sendto(transFd, message, INPUT_MAX, 0, (struct sockaddr*) &transAddr, transLen) == -1) {
            printf("%s: failed to send file open error\n", prog);
        }
        return 0;
    }
    memset(message, 0, INPUT_MAX);

    fseek(fPtr, 0, SEEK_END);
    fSize = ftell(fPtr);
    fseek(fPtr, 0, SEEK_SET);

    if (sendto(transFd, &fSize, sizeof(fSize), 0, (struct sockaddr*) &transAddr, transLen) == -1) {
        printf("%s: failed to send file size\n", prog);
        return 0;
    }
    if (recvfrom(transFd, message, INPUT_MAX, 0, (struct sockaddr*) &transAddr, &transLen) == -1) {
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
        tAmount = fSize > INPUT_MAX ? INPUT_MAX : fSize;
        sSize = sendto(transFd, message, tAmount, 0, (struct sockaddr*) &transAddr, transLen);
        if (sSize == -1) {
            printf("%s: failed to transmit the whole file\n", prog);
            return 0;
        }

        memset(message, 0, INPUT_MAX);
        fSize -= sSize;
    }
    printf("%s: file successfully transmitted\n", prog);

    fclose(fPtr);
    return 1;
}


char* tcp_array_receive(char* prog, int recvFd) {
    char* message;
    char* fDest;
    long int fSize;
    long int rSize;
    long int cPos;
    long int rAmount;

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        printf("%s: failed to allocate necessary memory\n", prog);
        return NULL;
    }

    if (recv(recvFd, &fSize, sizeof(fSize), 0) == -1) {
        printf("%s: failed to receive file size\n", prog);
        return NULL;
    }

    fDest = calloc(fSize, sizeof(char));
    if (fDest == NULL) {
        printf("%s: failed to allocate necessary memory\n", prog);
        return NULL;
    }

    sprintf(message, "%s", "ready");
    if (send(recvFd, message, INPUT_MAX, 0) == -1) {
        printf("%s: failed to send file size response\n", prog);
        return NULL;
    }
    memset(message, 0, INPUT_MAX);

    printf("%s: receiving the file...\n", prog);
    cPos = 0;
    while (fSize > 0) {
        rAmount = fSize > INPUT_MAX ? INPUT_MAX : fSize;
        rSize = recv(recvFd, &fDest[cPos], rAmount, 0);
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


char* udp_array_receive(char* prog, int recvFd, struct sockaddr_storage recvAddr, socklen_t recvLen) {
    char* message;
    char* fDest;
    long int fSize;
    long int rSize;
    long int cPos;
    long int rAmount;

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        printf("%s: failed to allocate necessary memory\n", prog);
        return NULL;
    }

    if (recvfrom(recvFd, &fSize, sizeof(fSize), 0, (struct sockaddr*) &recvAddr, &recvLen) == -1) {
        printf("%s: failed to receive file size\n", prog);
        return NULL;
    }

    fDest = calloc(fSize, sizeof(char));
    if (fDest == NULL) {
        printf("%s: failed to allocate necessary memory\n", prog);
        return NULL;
    }

    sprintf(message, "%s", "ready");
    if (sendto(recvFd, message, INPUT_MAX, 0, (struct sockaddr*) &recvAddr, recvLen) == -1) {
        printf("%s: failed to send file size response\n", prog);
        return NULL;
    }
    memset(message, 0, INPUT_MAX);

    printf("%s: receiving the file...\n", prog);
    cPos = 0;
    while (fSize > 0) {
        rAmount = fSize > INPUT_MAX ? INPUT_MAX : fSize;
        rSize = recvfrom(recvFd, &fDest[cPos], rAmount, 0, (struct sockaddr*) &recvAddr, &recvLen);
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


int tcp_array_transmit(char* prog, int transFd, char* fDest) {
    char* message;
    long int fSize;
    long int sSize;
    long int cPos;
    long int tAmount;

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        printf("%s: failed to allocate necessary memory\n", prog);
        return 0;
    }

    fSize = strlen(fDest);

    if (send(transFd, &fSize, sizeof(fSize), 0) == -1) {
        printf("%s: failed to send file size\n", prog);
        return 0;
    }
    if (recv(transFd, message, INPUT_MAX, 0) == -1) {
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
        tAmount = fSize > INPUT_MAX ? INPUT_MAX : fSize;
        sSize = send(transFd, &fDest[cPos], tAmount, 0);
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


int udp_array_transmit(char* prog, int transFd, char* fDest, struct sockaddr_storage transAddr, socklen_t transLen) {
    char* message;
    long int fSize;
    long int sSize;
    long int cPos;
    long int tAmount;

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        printf("%s: failed to allocate necessary memory\n", prog);
        return 0;
    }

    fSize = strlen(fDest);

    if (sendto(transFd, &fSize, sizeof(fSize), 0, (struct sockaddr*) &transAddr, transLen) == -1) {
        printf("%s: failed to send file size\n", prog);
        return 0;
    }
    if (recvfrom(transFd, message, INPUT_MAX, 0, (struct sockaddr*) &transAddr, &transLen) == -1) {
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
        tAmount = fSize > INPUT_MAX ? INPUT_MAX : fSize;
        sSize = sendto(transFd, &fDest[cPos], tAmount, 0, (struct sockaddr*) &transAddr, transLen);
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
