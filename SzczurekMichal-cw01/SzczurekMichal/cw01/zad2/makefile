CC = gcc -Wall
res = raport2.txt
static:
	make clean
	$(CC) -c lib1.c
	ar crs liblib1.a lib1.o
	$(CC) main.c -o main -L. -llib1
	
shared:
	make clean
	$(CC) -c lib1.c
	$(CC) -shared -fPIC -o liblib1.so lib1.o
	$(CC) main.c -o main -L. -llib1 -Wl,-rpath=`pwd`

test_static:
	echo '\n >>>>>>> STATIC TEST: \n' >> $(res)
	make static
	make test

test_shared:
	echo '\n >>>>>>> SHARED TEST: \n' >> $(res)
	make shared
	make test
		
test:
	./main test 5 2 3 small.txt:small.txt $(res)
	./main test 5 2 3 medium.txt:medium.txt $(res)
	./main test 5 2 3 big.txt:big.txt $(res)
	./main test 20 5 5 medium.txt:medium.txt $(res)
	./main test 100 10 10 medium.txt:medium.txt $(res)

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f main

