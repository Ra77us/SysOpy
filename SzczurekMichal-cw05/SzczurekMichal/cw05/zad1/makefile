CC = gcc -Wall -g

all: 
	make clean
	$(CC) main.c -o main

test:
	make all
	./main test.txt
clean:
	rm -f *.o
	rm -f main
