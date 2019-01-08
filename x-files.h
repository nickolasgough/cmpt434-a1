/* Nickolas Gough, nvg081, 11181823 */


int file_exists(char* path);
int file_read(char* path, int (*func)(char* r));
int file_append(char* path, char* text);
