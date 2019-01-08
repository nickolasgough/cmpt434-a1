/* Nickolas Gough, nvg081, 11181823 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define INPUT_MAX 10


int file_exists(char* path) {
    return access(path, F_OK) == 0;
}


int file_read(char* path, int (*func)(char* r)) {
    FILE* fptr;
    char* temp;

    fptr = fopen(path, "r");
    if (fptr == NULL) {
        return 0;
    }

    temp = calloc(INPUT_MAX, sizeof(char));
    if (temp == NULL) {
        return 0;
    }
    while (fread(temp, sizeof(char), INPUT_MAX, fptr) > 0) {
        if (!func(temp)) {
            return -1;
        }
        memset(temp, 0, INPUT_MAX);
    }

    fclose(fptr);
    return 1;
}


int file_append(char* path, char* text) {
    FILE* fptr;

    fptr = fopen(path, "a");
    if (fptr == NULL) {
        return 0;
    }

    if (fwrite(text, sizeof(char), strlen(text), fptr) < 0) {
        return -1;
    }

    fclose(fptr);
    return 1;
}
