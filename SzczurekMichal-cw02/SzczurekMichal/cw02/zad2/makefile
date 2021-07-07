CC = gcc -Wall -g

lib:
	make clean
	$(CC) main.c -o main
sys:
	make clean
	$(CC) main.c -o main -D SYS
	
lib_time:
	make clean
	$(CC) main.c -o main -D TIME

sys_time:
	make clean
	$(CC) main.c -o main -D SYS -D TIME
	
tests:
	make lib_time
	echo "=== Test for lib version ===\n" > pomiar_zad_2.txt
	make test
	make sys_time
	echo "=== Test for sys version ===\n" >> pomiar_zad_2.txt
	make test
	
test:	
	./main A small.txt
	./main B small.txt
	./main . small.txt
	./main A medium.txt
	./main B medium.txt
	./main . medium.txt
	./main A big.txt
	./main B big.txt
	./main . big.txt
clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f main
