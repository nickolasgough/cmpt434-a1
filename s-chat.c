/* Nickolas Gough 11181823 nvg081
   Mike Conly 11240933 mjc579 */


#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#include <fcntl.h>
#include <string.h>

#include <standards.h>
#include <os.h>

#include "list.h"


#define STD_IN 0
#define STD_OUT 1

#define MESSAGE_MAX 1000

#define PORT_MIN 30001
#define PORT_MAX 40000


/* The incoming and outgoing messages */
LIST* incoming;
LIST* outgoing;

/* Synchronization semaphores to mediate access */
int inSem;
int outSem;

typedef struct SockInfo {
    char* mName;
    int fd;
    struct sockaddr* addr;
} SockInfo;


/* Determine if the given port is within the allowable range */
int in_range(char* p) {
    int port = atoi(p);
    
    return port < PORT_MIN || port > PORT_MAX ? -1 : 0;
}


/* Unblock the given file descriptor */
int unblock_fd(int fd) {
    int flags = fcntl(fd, F_GETFL);
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return -1;
    }
    
    return flags;
}


/* Block the given file descriptor */
int block_fd(int fd, int flags) {
    return fcntl(fd, F_SETFL, flags);
}


/* Return a string of the current time */
char* append_time(char* message) {
    char* newMsg = calloc(MESSAGE_MAX, sizeof(char));
    struct timeval t;
    
    if (newMsg == NULL) {
        printf("s-chat: error - failed to allocate time to extract the current time\n");
        exit(1);
    }
    
    gettimeofday(&t, NULL);
    if (sprintf(newMsg, "%s (%ld.%ld)", message, t.tv_sec, t.tv_usec) < 0) {
        printf("s-chat: error - failed to extract the current time in seconds and microseconds\n");
        exit(1);
    }
    free(message);
    
    return newMsg;
}


/* Connect socket to the given machine and port
   return the file descriptor to the socket */
SockInfo socket_fd(char* mName, char* port, int b) {
    char* hName = calloc(MESSAGE_MAX, sizeof(char));
    struct addrinfo* pAi;
    struct addrinfo hints;
    int sockFd;
    int status;
    SockInfo sockInfo;
    
    if (port == NULL) {
        printf("socket_fd received NULL machine name or port\n");
        exit(1);
    }
    if (hName == NULL) {
        printf("s-chat: error - failed to allocate memory to establish a connection\n");
        exit(1);
    }
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    if (mName == NULL) {
        hints.ai_flags = AI_PASSIVE;
    }

    if (strcmp(mName, "raspberrypi") == 0) {
        sprintf(mName, "%s", "raspi1");
    }
    if (sprintf(hName, "%s.usask.ca", mName) < 0) {
        printf("s-chat: error - failed to determine the hostname of the given machine\n");
        exit(1);
    }
    
    status = getaddrinfo(hName, port, &hints, &pAi);
    if (status != 0) {
        printf("s-chat: error - could not connect to %s on port %s\n", hName, port);
        exit(1);
    }
    
    sockFd = socket(pAi->ai_family, pAi->ai_socktype, pAi->ai_protocol);
    if (sockFd < 0) {
        printf("s-chat: error - failed to establish connection with %s on port %s\n", hName, port);
        exit(1);
    }
    
    if (b) {
        if (bind(sockFd, pAi->ai_addr, pAi->ai_addrlen) == -1) {
            printf("s-chat: error - failed to bind the connection with %s on port %s\n", hName, port);
            exit(1);
        }
    }
    
    sockInfo.mName = mName;
    sockInfo.fd = sockFd;
    sockInfo.addr = pAi->ai_addr;
    return sockInfo;
}


/* The main function for collecting and sharing input */
PROCESS in_main() {
    char* message = calloc(MESSAGE_MAX, sizeof(char));
    char* prsdMsg;
    int flags;
    
    if (message == NULL) {
        printf("s-chat: error - could not allocate memory to collect input\n");
        exit(1);
    }

#ifdef DEBUG
    printf("Got to in_main with: no arguments\n");
#endif
    
    flags = unblock_fd(STD_IN);
    if (flags == -1) {
        printf("s-chat: error - could not unblock the standard input file descriptor\n");
        exit(1);
    }
    while (1) {        
        if (read(STD_IN, message, MESSAGE_MAX) != -1) {
            prsdMsg = strtok(message, "\n");
            if (prsdMsg == NULL) {
                continue;
            }

            P(outSem);
#ifdef DEBUG
            printf("in_main acquired the lock to the outgoing messages\n");
#endif
            ListPrepend(outgoing, (void*) prsdMsg);
#ifdef DEBUG
            printf("in_main is releasing the lock to the outgoing messages\n");
#endif
            V(outSem);
            
            if (strcmp(prsdMsg, "goodbye") == 0) {
                while (1) {
                    P(outSem);
                    if (ListCount(outgoing) == 0) {
                        exit(0);
                    }
                    V(outSem);
                }
            }
            
            message = calloc(MESSAGE_MAX, sizeof(char));
            if (message == NULL) {
                printf("s-chat: error - could not allocate memory to collect input\n");
                exit(1);
            }
        }
    }
    if (block_fd(STD_IN, flags) == -1) {
        printf("s-chat: error - could not block the standard input file descriptor\n");
        exit(1);
    }
}


/* The main function for collecting and sharing output */
PROCESS out_main(void* pMName) {
    char* mName;
    char* message;
    int flags;
    
    if (pMName == NULL) {
        printf("out_main received NULL machine name\n");
        exit(1);
    }
    mName = ((char*) pMName);

#ifdef DEBUG
    printf("Got to out_main with: mName = %s\n", mName);
#endif
    
    flags = unblock_fd(STD_OUT);
    if (flags == -1) {
        printf("s-chat: error - could not unblock the standard output file descriptor\n");
        exit(1);
    }
    while (1) {
        P(inSem);
        while (ListCount(incoming) > 0) {
#ifdef DEBUG
            printf("out_main acquired the lock to the incoming messages\n");
#endif
            message = (char*) ListTrim(incoming);

            if (message != NULL) {
                printf("%s -> %s\n", mName, message);
                free(message);
                message = NULL;
            }

#ifdef DEBUG
            printf("out_main is releasing the lock to the incoming messages\n");
#endif
        }
        V(inSem);
    }
    if (block_fd(STD_OUT, flags) == -1) {
        printf("s-chat: error - could not block the standard output file descriptor\n");
        exit(1);
    }
}


/* The main function for sending messages to the given socket */
PROCESS send_main(void* pSockInfo) {
    SockInfo sockPeer;
    char* message;
    int msgLen;
    uint16_t netLen;
    int flags;

    if (pSockInfo == NULL) {
        printf("send_main received NULL socket file descriptor\n");
        exit(1);
    }
    
    sockPeer = *((SockInfo*) pSockInfo);
    
#ifdef DEBUG
    printf("Got to send_main with: fd = %d, mName = %s\n", sockPeer.fd, sockPeer.mName);
#endif
    
    flags = unblock_fd(sockPeer.fd);
    if (flags == -1) {
        printf("s-chat: error - could not unblock the socket's file descriptor\n");
        exit(1);
    }
    while (1) {
        P(outSem);
        while (ListCount(outgoing) > 0) {
#ifdef DEBUG
            printf("send_main acquired the lock to the outgoing messages\n");
#endif
            message = (char*) ListTrim(outgoing);
            if (message != NULL) {
                message = append_time(message);
                msgLen = strlen(message);
                netLen = htons(msgLen);
#ifdef DEBUG
                printf("send_main is sending a message of length %d\n", msgLen);
#endif
                if (sendto(sockPeer.fd, (void*) &netLen, sizeof(netLen), 0, sockPeer.addr, sizeof(*(sockPeer.addr))) == -1) {
                    printf("s-chat: error - failed to send the length %d", msgLen);
                }

                if (sendto(sockPeer.fd, (void*) message, msgLen, 0, sockPeer.addr, sizeof(*(sockPeer.addr))) == -1) {
                    printf("s-chat: error - failed to send the message %s", message);
                }
                free(message);
                message = NULL;
            }

#ifdef DEBUG
            printf("send_main is releasing the lock to the outgoing messages\n");
#endif
        }
        V(outSem);
    }
    if (block_fd(sockPeer.fd, flags) == -1) {
        printf("s-chat: error - could not block the socket's file descriptor\n");
        exit(1);
    }
}


/* The main function for receiving messages from the given socket */
PROCESS receive_main(void* pSockInfo) {
    SockInfo sockHost;
    int flags;
    char* message = calloc(MESSAGE_MAX, sizeof(char));
    int msgLen;
#ifdef DEBUG
    uint16_t hostLen;
#endif
    struct sockaddr from;
    socklen_t fromLen;
    
    if (message == NULL) {
        printf("s-chat: error - could not allocate memory to receive incoming messages\n");
        exit(1);
    }
    
    sockHost = *((SockInfo*) pSockInfo);
    
#ifdef DEBUG
    printf("Got to receive_main with: fd = %d, mName = %s\n", sockHost.fd, sockHost.mName);
#endif
    
    flags = unblock_fd(sockHost.fd);
    if (flags == -1) {
        printf("s-chat: error - could not unblock the socket's file descriptor\n");
        exit(1);
    }
    while (1) {
        if (recvfrom(sockHost.fd, (void*) &msgLen, sizeof(msgLen), 0, &from, &fromLen) != -1) {
            while (recvfrom(sockHost.fd, (void*) message, MESSAGE_MAX, 0, &from, &fromLen) == -1) {}
            
            P(inSem);
#ifdef DEBUG
            hostLen = ntohs(msgLen);
            printf("receive_main acquired the lock to the incoming messages\n");
            printf("receive_main received a message of length %d\n", hostLen);
#endif
            ListPrepend(incoming, (void*) message);
#ifdef DEBUG
            printf("receive_main is releasing the lock to the incoming messages\n");
#endif
            V(inSem);
            
            message = calloc(MESSAGE_MAX, sizeof(char));
            if (message == NULL) {
                printf("s-chat: error - could not allocate memory to collect input\n");
                exit(1);
            }
        }
    }
    if (block_fd(sockHost.fd, flags) == -1) {
        printf("s-chat: error - could not block the socket's file descriptor\n");
        exit(1);
    }
}


/* The main function for establishing the connections and spawning threads */
int mainp(int argc, char* argv[]) {
    char* mName = calloc(MESSAGE_MAX, sizeof(char));
    char* ip;
    char* ep;
    int nArgs = argc-1;
    SockInfo* pSockHost = calloc(1, sizeof(SockInfo));
    SockInfo* pSockPeer = calloc(1, sizeof(SockInfo));
    char* hostname = calloc(MESSAGE_MAX, sizeof(char));

    if (nArgs != 3) {
        printf("usage: s-chat <internal port> <machine name> <external port>\n");
        exit(1);
    }

    ip = argv[1];
    mName = argv[2];
    ep = argv[3];
    if (in_range(ip) != 0 || in_range(ep) != 0) {
        printf("s-chat: error - each port must be between %d and %d, inclusive\n", PORT_MIN, PORT_MAX);
        exit(1);
    }

    if (hostname == NULL || pSockHost == NULL || pSockPeer == NULL) {
        printf("s-chat: error - could not allocate memory for the socket file descriptors\n");
        exit(1);
    }

    if (gethostname(hostname, MESSAGE_MAX) == -1) {
        printf("s-chat: error - could not determine the hostname of the host machine\n");
        exit(1);
    }
    *pSockHost = socket_fd(hostname, ip, 1);
    *pSockPeer = socket_fd(mName, ep, 0);

    incoming = ListCreate();
    outgoing = ListCreate();
    if (incoming == NULL || outgoing == NULL) {
        printf("s-chat: error - could not allocate space for messages\n");
        exit(1);
    }

    inSem = NewSem(1);
    outSem = NewSem(1);
    if (inSem == -1 || outSem == -1) {
        printf("s-chat: error - could not allocate space for synchronization\n");
        exit(1);
    }

#ifdef DEBUG
    printf("main received arguments ip = %s, mName = %s, and ep = %s\n", ip, mName, ep);
#endif
    
    Create(&out_main, 128000, NULL, (void*) mName, NORM, USR);
    Create(&in_main, 128000, NULL, NULL, NORM, USR);
    Create(&receive_main, 128000, NULL, (void*) pSockHost, NORM, USR);
    Create(&send_main, 128000, NULL, (void*) pSockPeer, NORM, USR);

    return 0;
}
