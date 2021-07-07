CC = gcc -Wall
all:
	make clean
	$(CC) main.c -o main
	$(CC) exec.c -o exec

test:
	make all
	./main ignore fork 
	./main ignore exec
	./main mask fork 
	./main mask exec
	./main pending fork 
	./main pending exec
	./main handler fork 
	./main handler exec

clean:
	rm -f *.o
	rm -f main
	rm -f exec

