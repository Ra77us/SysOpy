CC = gcc -Wall 

all: 
	make clean
	$(CC) main.c -o main

test:
	make all
	./main data
	./main nadawca
	./main root Subject text
clean:
	rm -f *.o
	rm -f main
