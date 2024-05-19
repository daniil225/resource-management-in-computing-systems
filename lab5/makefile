# Makefile for lab #5
all: main

main: main.o
	gcc main.o -o main
	
lab1: lab1.o
	gcc lab1.o -o lab1

lab2: lab2.o
	gcc lab2.o -o lab2

lab3: lab3.o
	gcc lab3.o -o lab3

lab4: lab4.o
	gcc lab4.o -o lab4

lab5: lab5.o
	gcc lab5.o -o lab5


main.o: main.c
	gcc -c main.c

lab1.o: lab1.c
	gcc  -c lab1.c

lab2.o: lab2.c
	gcc  -c lab2.c

lab3.o: lab3.c
	gcc  -c lab3.c

lab4.o: lab4.c
	gcc  -c lab4.c

lab5.o: lab5.c
	gcc  -c lab5.c


clean:
	rm -rf *.o main