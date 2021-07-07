CC = gcc -Wall -g
 
all: 
	make clean
	$(CC) client.c -o client -lrt
	$(CC) server.c -o server -lrt

clean:
	rm -f *.o
