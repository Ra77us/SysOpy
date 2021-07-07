CC = gcc -Wall -g

all: 
	make clean
	$(CC) shared.c main.c -o main
	$(CC) shared.c cook.c -o cook
	$(CC) shared.c deliverer.c -o deliverer

clean:
	rm -f cook
	rm -f main
	rm -f deliverer
