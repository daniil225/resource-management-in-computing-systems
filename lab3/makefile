# Makefile for lab #2
all: main

main: main.o
	gcc -std=c11 main.o -o main

main.o: main.c
	gcc -std=c11 -c main.c

clean:
	rm -rf *.o main