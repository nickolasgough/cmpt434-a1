# Nickolas Gough, nvg081, 11181823

GCC = gcc
FLAGS = -g -std=gnu90 -Wall -pedantic


all: x-client tcp-server


x-client: x-client.o x-sockets.o
	$(GCC) -o $@ $(FLAGS) $^

x-client.o: x-client.c
	$(CC) -o $@ -c $(FLAGS) $<


tcp-proxy: tcp-proxy.o x-sockets.o
	$(GCC) -o $@ $(FLAGS) $^

tcp-proxy.o: tcp-proxy.c
	$(CC) -o $@ -c $(FLAGS) $<


tcp-server: tcp-server.o x-sockets.o
	$(GCC) -o $@ $(FLAGS) $^

tcp-server.o: tcp-server.c
	$(CC) -o $@ -c $(FLAGS) $<


x-sockets.o: x-sockets.c x-sockets.h
	$(CC) -o $@ -c $(FLAGS) $<


clean:
	rm -rf x-client tcp-server *.o
