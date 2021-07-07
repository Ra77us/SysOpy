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
	echo "=== Test for lib version ===\n" > pomiar_zad_1.txt
	make test
	make sys_time
	echo "=== Test for sys version ===\n" >> pomiar_zad_1.txt
	make test
	
test:	./main small_a.txt small_b.txt
	./main small_a.txt medium_b.txt
	./main small_a.txt big_b.txt
	./main medium_a.txt medium_b.txt
	./main medium_a.txt big_b.txt
	./main big_a.txt big_b.txt
	
	
clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f main
