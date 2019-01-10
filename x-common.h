/* Nickolas Gough, nvg081, 11181823 */


#define STD_IN 0

#define PORT_MIN 30000
#define PORT_MAX 40000

#define INPUT_MAX 1000


int check_port(char* port);

int tcp_client_get(char* prog, int serverFd, char* lFile, char* rFile);
int tcp_client_put(char* prog, int serverFd, char* lFile, char* rFile);

int tcp_server_get(char* prog, int clientFd, char* fName, int eCheck);
int tcp_server_put(char* prog, int clientFd, char* fName, int eCheck);
