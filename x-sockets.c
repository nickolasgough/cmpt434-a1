/* Nickolas Gough, nvg081, 11181823 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>


#define INPUT_MAX 1000


int tcp_socket(int* outFd, struct addrinfo* outInfo, char* mName, char* port) {
    char* hName;
    struct addrinfo* pAi;
    struct addrinfo hints;
    int sockFd;
    
    hName = calloc(INPUT_MAX, sizeof(char));
    if (hName == NULL || port == NULL) {
        return 0;
    }
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    sprintf(hName, "%s.usask.ca", mName);
    
    if (getaddrinfo(hName, port, &hints, &pAi) != 0) {
        return 0;
    }
    if (pAi->ai_socktype != SOCK_STREAM || pAi->ai_protocol != IPPROTO_TCP) {
        return 0;
    }
    
    sockFd = socket(pAi->ai_family, pAi->ai_socktype, IPPROTO_TCP);
    if (sockFd < 0) {
        return 0;
    }
    
    *outFd = sockFd;
    *outInfo = *pAi;
    return 1;
}


int udp_socket(int* outFd, struct addrinfo* outInfo, char* mName, char* port) {
    char* hName;
    struct addrinfo* pAi;
    struct addrinfo hints;
    int sockFd;
    
    hName = calloc(INPUT_MAX, sizeof(char));
    if (hName == NULL || port == NULL) {
        return 0;
    }
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    sprintf(hName, "%s.usask.ca", mName);
    
    if (getaddrinfo(hName, port, &hints, &pAi) != 0) {
        return 0;
    }
    if (pAi->ai_socktype != SOCK_DGRAM || pAi->ai_protocol != IPPROTO_UDP) {
        return 0;
    }
    
    sockFd = socket(pAi->ai_family, pAi->ai_socktype, IPPROTO_UDP);
    if (sockFd < 0) {
        return 0;
    }
    
    *outFd = sockFd;
    *outInfo = *pAi;
    return 1;
}
