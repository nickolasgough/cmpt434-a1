/* Nickolas Gough, nvg081, 11181823 */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "x-files.h"


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

void handle_fault(int err) {
    if (err == 1) {
        printf("usage: x-client <host name> <port number>\n");
        exit(err);
    }
    if (err == 2) {
        printf("x-client: failed to allocate memory\n");
        exit(err);
    }
    if (err == 3) {
        printf("x-client: failed to read command\n");
    }
    if (err == 4) {
        printf("x-client: failed to put the file\n");
    }
    if (err == 5) {
        printf("x-client: the file does not exist\n");
    }
}


void get_file(char* lfile, char* rfile) {
    printf("Getting file: %s, %s\n", lfile, rfile);
}


int send_file(char* fpart) {
    printf("%s\n", fpart);
    return 1;
}


void put_file(char* lfile, char* rfile) {
    if (!file_exists(lfile)) {
        handle_fault(5);
        return;
    }
    if (!file_read(lfile, send_file)) {
        handle_fault(4);
    }
}


int main(int argc, char* argv[]) {
    char cmd[INPUT_MAX];
    char lfile[INPUT_MAX];
    char rfile[INPUT_MAX];
    char* hname;
    char* portn;

    if (argc != 3) {
        handle_fault(1);
    }

    hname = argv[1];
    portn = argv[2];
    if (!check_port(portn)) {
        handle_fault(1);
    }

    while (1) {
        printf("x-client ? ");
        if (scanf("%s", cmd) < 0) {
            handle_fault(3);
        }
        if (strcmp(cmd, "quit") == 0) {
            exit(0);
        }
        if (scanf("%s %s", lfile, rfile) < 0) {
            handle_fault(3);
        }
        
        if (strcmp(cmd, "get") == 0) {
            get_file(lfile, rfile);
        }
        else if (strcmp(cmd, "put") == 0) {
            put_file(lfile, rfile);
        }
        else {
            printf("Unknown command.\n");
            printf("Known commands:\n");
            printf("get <local file> <remote file>\n");
            printf("put <local file> <remote file>\n");
            printf("quit\n");
        }
    }

    exit(0);
}
