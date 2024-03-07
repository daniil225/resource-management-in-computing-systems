# Makefile for lab #2
all: main

main: main.o
	gcc -std=c11 main.o -o main

main.o: main.c
	gcc -std=c11 -c main.c -lm -O2

clean:
	rm -rf *.o main