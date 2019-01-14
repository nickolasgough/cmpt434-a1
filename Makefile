# Nickolas Gough, nvg081, 11181823

GCC = gcc
FLAGS = -g -std=gnu90 -Wall -pedantic


all: x-client tcp-proxy tcp-server


x-client: x-client.o x-sockets.o x-common.o
	$(GCC) -o $@ $(FLAGS) $^

x-client.o: x-client.c
	$(CC) -o $@ -c $(FLAGS) $<


tcp-proxy: tcp-proxy.o x-sockets.o x-common.o
	$(GCC) -o $@ $(FLAGS) $^

tcp-proxy.o: tcp-proxy.c
	$(CC) -o $@ -c $(FLAGS) $<


mixed-proxy: mixed-proxy.o x-sockets.o x-common.o
	$(GCC) -o $@ $(FLAGS) $^

mixed-proxy.o: mixed-proxy.c
	$(CC) -o $@ -c $(FLAGS) $<


tcp-server: tcp-server.o x-sockets.o x-common.o
	$(GCC) -o $@ $(FLAGS) $^

tcp-server.o: tcp-server.c
	$(CC) -o $@ -c $(FLAGS) $<


udp-server: udp-server.o x-sockets.o x-common.o
	$(GCC) -o $@ $(FLAGS) $^

udp-server.o: udp-server.c
	$(CC) -o $@ -c $(FLAGS) $<


x-sockets.o: x-sockets.c x-sockets.h
	$(CC) -o $@ -c $(FLAGS) $<

x-common.o: x-common.c x-common.h
	$(CC) -o $@ -c $(FLAGS) $<


clean:
	rm -rf x-client tcp-proxy tcp-server *.o
