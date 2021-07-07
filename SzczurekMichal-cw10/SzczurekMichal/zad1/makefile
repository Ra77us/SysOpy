CC = gcc -Wall -g -pthread

all: 
	make clean
	make server
	make client

client:
	$(CC) -o client client.c

server:
	$(CC) -o server server.c

clean:
	rm -f  client server socket
