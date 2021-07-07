CC = gcc -Wall -std=c11 -D_XOPEN_SOURCE=700
N := 1

all:
	make clean
	$(CC) consumer.c -o consumer
	$(CC) producer.c -o producer
	$(CC) main.c -o main
	$(CC) validator.c -o validator


clean:
	rm -f main producer consumer check FIFO_ID
	
test:
	make  -s test_wrapper >> res.txt

test_wrapper:
	
	echo "\ntests for N=5"
	make  N=5 test_one_one
	echo "\noutput: \n"
	cat output_file.txt
	make -s N=5 test_one_many
	echo "\noutput: \n"
	cat output_file.txt
	make -s N=5 test_many_one
	echo "\noutput: \n"
	cat output_file.txt
	make -s N=5 test_many_many
	echo "\noutput: \n"
	cat output_file.txt
	echo "\ntests for N=25"
	make -s N=30 test_one_one
	echo "\noutput: \n"
	cat output_file.txt
	make -s  N=30 test_one_many
	echo "\noutput: \n"
	cat output_file.txt
	make -s  N=30 test_many_one
	echo "\noutput: \n"
	cat output_file.txt
	make -s  N=30 test_many_many
	echo "\noutput: \n"
	cat output_file.txt
	echo "\ntests for N=5000"
	echo "\n(No output preview for big data)\n"
	make  N=5000 test_one_one_b
	make  N=5000 test_one_many_b
	make  N=5000 test_many_one_b
	make  N=5000 test_many_many_b	
	echo "\ntest in different terminals:"
	make -s test_terminals
	
test_one_one:
	echo "\n=========================" 
	echo "1 producer 1 consumer"
	./main 1 1 $(N)
	./validator producer_file_1.txt output_file.txt 1

test_one_many:
	echo "\n========================="
	echo "1 producer 5 consumers"
	./main 1 5 $(N)
	./validator producer_file_1.txt output_file.txt 1 

test_many_one:
	echo "\n========================="
	echo "5 producers 1 consumer"
	./main 5 1 $(N)
	./validator producer_file_1.txt output_file.txt 1 
	./validator producer_file_2.txt output_file.txt 2
	./validator producer_file_3.txt output_file.txt 3 
	./validator producer_file_4.txt output_file.txt 4 
	./validator producer_file_5.txt output_file.txt 5 

test_many_many:
	echo "\n========================="
	echo "5 producentów 5 consumers"
	./main 5 1 $(N)
	./validator producer_file_1.txt output_file.txt 1 
	./validator producer_file_2.txt output_file.txt 2
	./validator producer_file_3.txt output_file.txt 3 
	./validator producer_file_4.txt output_file.txt 4 
	./validator producer_file_5.txt output_file.txt 5 
	
	
test_one_one_b:
	echo "\n========================="
	echo "1 producer 1 consumer"
	./main 1 1 $(N) b
	./validator producer_file_big_1.txt  output_file.txt 1 

test_one_many_b:
	echo "\n========================="
	echo "1 producer 5 consumers"
	./main 1 5 $(N) b
	./validator producer_file_big_1.txt output_file.txt 1 

test_many_one_b:
	echo "\n========================="
	echo "5 producers 1 consumer"
	./main 5 1 $(N) b
	./validator producer_file_big_1.txt output_file.txt 1 
	./validator producer_file_big_2.txt output_file.txt 2
	./validator producer_file_big_3.txt output_file.txt 3 
	./validator producer_file_big_4.txt output_file.txt 4 
	./validator producer_file_big_5.txt output_file.txt 5 

test_many_many_b:
	echo "\n========================="
	echo "5 producentów 5 consumers"
	./main 5 5 $(N) b
	./validator producer_file_big_1.txt output_file.txt 1 
	./validator producer_file_big_2.txt output_file.txt 2
	./validator producer_file_big_3.txt output_file.txt 3 
	./validator producer_file_big_4.txt output_file.txt 4 
	./validator producer_file_big_5.txt output_file.txt 5 

test_terminals:
	rm -f FIFO_ID
	mkfifo "FIFO_ID"
	gnome-terminal -e "bash -c \"./consumer FIFO_ID output_file.txt 5; exec bash\""
	gnome-terminal -e "bash -c \"./producer FIFO_ID 1 producer_file_1.txt 5; exec bash\""
	gnome-terminal -e "bash -c \"./producer FIFO_ID 2 producer_file_2.txt 5; exec bash\""
	gnome-terminal -e "bash -c \"./producer FIFO_ID 3 producer_file_3.txt 5; exec bash\""
	gnome-terminal -e "bash -c \"./producer FIFO_ID 4 producer_file_4.txt 5; exec bash\""
	gnome-terminal -e "bash -c \"./producer FIFO_ID 5 producer_file_5.txt 5; exec bash\""
	sleep 25
	./validator producer_file_1.txt output_file.txt 1 
	./validator producer_file_2.txt output_file.txt 2
	./validator producer_file_3.txt output_file.txt 3 
	./validator producer_file_4.txt output_file.txt 4 
	./validator producer_file_5.txt output_file.txt 5 

