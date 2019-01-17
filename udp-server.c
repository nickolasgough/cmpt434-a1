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
#include "x-common.h"


void get_file(int hostFd, struct sockaddr_storage clientAddr, socklen_t clientLen) {
    char* message;
    char* fName;

    message = calloc(INPUT_MAX, sizeof(char));
    fName = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL || fName == NULL) {
        return;
    }

    sprintf(message, "%s", "ready");
    if (sendto(hostFd, message, INPUT_MAX, 0, (struct sockaddr*) &clientAddr, clientLen) == - 1) {
        printf("udp-server: failed to send get file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    if (recvfrom(hostFd, fName, INPUT_MAX, 0, (struct sockaddr*) &clientAddr, &clientLen) == -1) {
        printf("udp-server: failed to receive get file name\n");
        return;
    }

    if (access(fName, F_OK)) {
        printf("udp-server: file %s does not exist\n", fName);
        sprintf(message, "%s", "error");
        if (sendto(hostFd, message, INPUT_MAX, 0, (struct sockaddr*) &clientAddr, clientLen) == -1) {
            printf("udp-server: failed to send file does not exist error\n");
        }
        return;
    }
    memset(message, 0, INPUT_MAX);

    sprintf(message, "%s", "ready");
    if (sendto(hostFd, message, INPUT_MAX, 0, (struct sockaddr*) &clientAddr, clientLen) == -1) {
        printf("udp-server: failed to send get file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    udp_file_transmit("udp-server", hostFd, fName, clientAddr, clientLen);
}


void put_file(int hostFd, struct sockaddr_storage clientAddr, socklen_t clientLen)  {
    char* message;
    char* fName;

    message = calloc(INPUT_MAX, sizeof(char));
    fName = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL || fName == NULL) {
        printf("udp-server: failed to allocate necessary memory\n");
        return;
    }

    sprintf(message, "%s", "ready");
    if (sendto(hostFd, message, INPUT_MAX, 0, (struct sockaddr*) &clientAddr, clientLen) == - 1) {
        printf("udp-server: failed to send put file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    if (recvfrom(hostFd, fName, INPUT_MAX, 0, (struct sockaddr*) &clientAddr, &clientLen) == -1) {
        printf("udp-server: failed to receive put file name\n");
        return;
    }

    if (!access(fName, F_OK)) {
        printf("udp-server: file %s already exists\n", fName);
        sprintf(message, "%s", "error");
        if (sendto(hostFd, message, INPUT_MAX, 0, (struct sockaddr*) &clientAddr, clientLen) == -1) {
            printf("udp-server: failed to send file exists error\n");
        }
        return;
    }
    memset(message, 0, INPUT_MAX);

    sprintf(message, "%s", "ready");
    if (sendto(hostFd, message, INPUT_MAX, 0, (struct sockaddr*) &clientAddr, clientLen) == - 1) {
        printf("udp-server: failed to send put file name response\n");
        return;
    }
    memset(message, 0, INPUT_MAX);

    udp_file_receive("udp-server", hostFd, fName, clientAddr, clientLen);
}


int main(int argc, char* argv[]) {
    char* hName;
    char* hPort;
    int hostFd;
    struct addrinfo hostInfo;
    struct sockaddr_storage clientAddr;
    socklen_t clientLen;
    char* message;
    int rSize;

    char* fName;

    FILE* fPtr;
    long int fSize;
    long int sSize;
    long int tAmount;

    if (argc != 2) {
        printf("usage: ./udp-server <host port>\n");
        exit(1);
    }

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

    if (gethostname(hName, INPUT_MAX) == -1) {
        printf("udp-server: failed to determine the name of the machine\n");
        exit(1);
    }
    if (!udp_socket(&hostFd, &hostInfo, hName, hPort)) {
        printf("udp-server: failed to create udp socket for given host\n");
        exit(1);
    }
    if (bind(hostFd, hostInfo.ai_addr, hostInfo.ai_addrlen) == -1) {
        printf("udp-server: failed to bind udp socket for given host\n");
        exit(1);
    }

    message = calloc(INPUT_MAX, sizeof(char));
    if (message == NULL) {
        printf("udp-server: failed to allocate necessary memory\n");
        exit(1);
    }

    while (1) {
        rSize = recvfrom(hostFd, message, INPUT_MAX, 0, (struct sockaddr*) &clientAddr, &clientLen);
        if (rSize == -1) {
            printf("udp-server: failed to receive command from client\n");
            exit(1);
        }

        if (strcmp(message, "get") == 0) {
            /* This function cannot be invoked */
            /* get_file(hostFd, clientAddr, clientLen); */

            /* Copied and modified from get_file function */
            message = calloc(INPUT_MAX, sizeof(char));
            fName = calloc(INPUT_MAX, sizeof(char));
            if (message == NULL || fName == NULL) {
                continue;
            }

            sprintf(message, "%s", "ready");
            if (sendto(hostFd, message, INPUT_MAX, 0, (struct sockaddr*) &clientAddr, clientLen) == - 1) {
                printf("udp-server: failed to send get file name response\n");
                continue;
            }
            memset(message, 0, INPUT_MAX);

            if (recvfrom(hostFd, fName, INPUT_MAX, 0, (struct sockaddr*) &clientAddr, &clientLen) == -1) {
                printf("udp-server: failed to receive get file name\n");
                continue;
            }

            if (access(fName, F_OK)) {
                printf("udp-server: file %s does not exist\n", fName);
                sprintf(message, "%s", "error");
                if (sendto(hostFd, message, INPUT_MAX, 0, (struct sockaddr*) &clientAddr, clientLen) == -1) {
                    printf("udp-server: failed to send file does not exist error\n");
                }
                continue;
            }
            memset(message, 0, INPUT_MAX);

            sprintf(message, "%s", "ready");
            if (sendto(hostFd, message, INPUT_MAX, 0, (struct sockaddr*) &clientAddr, clientLen) == -1) {
                printf("udp-server: failed to send get file name response\n");
                continue;
            }
            memset(message, 0, INPUT_MAX);

            /* Copied and modified from udp_file_transmit function */
            message = calloc(INPUT_MAX, sizeof(char));
            if (message == NULL) {
                printf("udp-server: failed to allocate necessary memory\n");
                continue;
            }

            fPtr = fopen(fName, "r");
            if (fPtr == NULL) {
                printf("udp-server: failed to open the file\n");
                sprintf(message, "%s", "error");
                if (send(hostFd, message, INPUT_MAX, 0) == -1) {
                    printf("udp-server: failed to send file open error\n");
                }
                continue;
            }
            memset(message, 0, INPUT_MAX);

            fseek(fPtr, 0, SEEK_END);
            fSize = ftell(fPtr);
            fseek(fPtr, 0, SEEK_SET);

            if (send(hostFd, &fSize, sizeof(fSize), 0) == -1) {
                printf("udp-server: failed to send file size\n");
                continue;
            }
            if (recv(hostFd, message, INPUT_MAX, 0) == -1) {
                printf("udp-server: failed to receive file size response\n");
                continue;
            }
            if (strcmp(message, "ready") != 0) {
                printf("udp-server: received unexpected file size response\n");
                continue;
            }
            memset(message, 0, INPUT_MAX);

            printf("udp-server: transmitting the file...\n");
            while (fread(message, sizeof(char), INPUT_MAX, fPtr) > 0) {
                tAmount = fSize > INPUT_MAX ? INPUT_MAX : fSize;
                sSize = send(hostFd, message, tAmount, 0);
                if (sSize == -1) {
                    printf("udp-server: failed to transmit the whole file\n");
                    break;
                }

                memset(message, 0, INPUT_MAX);
                fSize -= sSize;
            }
            printf("udp-server: file successfully transmitted\n");

            fclose(fPtr);
        }
        if (strcmp(message, "put") == 0) {
            put_file(hostFd, clientAddr, clientLen);
        }
    }

    close(hostFd);
    exit(0);
}
