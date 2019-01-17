/* Nickolas Gough, nvg081, 11181823 */


#define STD_IN 0

#define PORT_MIN 30000
#define PORT_MAX 40000

#define INPUT_MAX 1000


int check_port(char* port);

char* proc_file(char* fDest);

int tcp_file_receive(char* prog, int serverFd, char* fName);
int udp_file_receive(char* prog, int recvFd, char* fName, struct sockaddr_storage recvAddr, socklen_t recvLen);

int tcp_file_transmit(char* prog, int clientFd, char* fName);
int udp_file_transmit(char* prog, int transFd, char* fName, struct sockaddr_storage transAddr, socklen_t transLen);

char* tcp_array_receive(char* prog, int serverFd);
char* udp_array_receive(char* prog, int recvFd, struct sockaddr* recvAddr, socklen_t recvLen);

int tcp_array_transmit(char* prog, int clientFd, char* fDest);
int udp_array_transmit(char* prog, int transFd, char* fDest, struct sockaddr_storage transAddr, socklen_t transLen);
