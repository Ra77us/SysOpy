CC = gcc -Wall -g

all: 
	make clean
	$(CC) shared.c main.c -o main -lpthread -lrt
	$(CC) shared.c cook.c -o cook -lpthread -lrt
	$(CC) shared.c deliverer.c -o deliverer -lpthread -lrt

clean:
	rm -f cook
	rm -f main
	rm -f deliverer
