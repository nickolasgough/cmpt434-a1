/* Nickolas Gough, nvg081, 11181823 */


#define STD_IN 0

#define PORT_MIN 30000
#define PORT_MAX 40000

#define INPUT_MAX 1000


int check_port(char* port);

int tcp_file_receive(char* prog, int serverFd, char* fName);
int tcp_file_transmit(char* prog, int clientFd, char* fName);

int tcp_array_receive(char* prog, int serverFd, char* fDest);
int tcp_array_transmit(char* prog, int clientFd, char* fDest);
