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
	echo "=== Test for lib version ===\n" > pomiar_zad_4.txt
	make test
	make sys_time
	echo "=== Test for sys version ===\n" >> pomiar_zad_4.txt
	make test
	
test:	
	./main small.txt res.txt 0000 ________
	./main medium.txt res.txt 0000 ________
	./main big.txt res.txt 0000 ________
	./main small.txt res.txt A ________
	./main medium.txt res.txt A ________
	./main big.txt res.txt A ________
	
clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f main
