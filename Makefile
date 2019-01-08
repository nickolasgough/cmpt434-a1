# Nickolas Gough, nvg081, 11181823

GCC = gcc
FLAGS = -g -std=gnu90 -Wall -pedantic


all: x-client

# Part 1
x-client: x-client.o x-files.o
	$(GCC) -o $@ $(FLAGS) $^

x-client.o: x-client.c
	$(CC) -o $@ -c $(FLAGS) $<

x-files.o: x-files.c x-files.h
	$(CC) -o $@ -c $(FLAGS) $<


clean:
	rm -rf x-client *.o