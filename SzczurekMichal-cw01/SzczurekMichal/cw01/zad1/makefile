CC = gcc -Wall

static:
	make clean
	$(CC) -c lib1.c
	ar crs liblib1.a lib1.o

shared:
	make clean
	$(CC) -c lib1.c
	$(CC) -shared -fPIC -o liblib1.so lib1.o

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f main
