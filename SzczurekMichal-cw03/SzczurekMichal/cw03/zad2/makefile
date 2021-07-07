CC = gcc -Wall -O$(level)
res = results.txt

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

dll:
	make clean
	$(CC) -c lib1.c
	$(CC) -shared -fPIC -o liblib1.so lib1.o
	$(CC) main.c -o main -L. -ldl -D DLL

test_optimization:
	echo "\n ******************** OPTIMIZATION TEST FOR O$(level) ******************** \n" >> $(res)
	make test_static
	make test_shared
	make test_dll

test_static:
	echo "\n >>>>>>> STATIC TEST: \n" >> $(res)
	make static
	make test

test_shared:
	echo "\n >>>>>>> SHARED TEST: \n" >> $(res)
	make shared
	make test

test_dll:
	echo "\n >>>>>>> DLL TEST: \n" >> $(res)
	make dll
	make test

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f main

test:
	./main $(res) -r 5 small.txt:small.txt 
	./main $(res) -r 5 medium.txt:medium.txt 
	./main $(res) -r 5 big.txt:big.txt 
	./main $(res) -r 20 medium.txt:medium.txt
	./main $(res) -r 100 medium.txt:medium.txt

