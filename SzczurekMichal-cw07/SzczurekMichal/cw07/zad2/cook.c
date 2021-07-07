
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "shared.h"

int shared_mem;

sem_t* oven_lock_sem;
sem_t* oven_size_sem;
sem_t* table_lock_sem;
sem_t* table_size_sem;
sem_t* table_empty_sem;
restaurant* pizzeria;

void sigint_handler(int signum){
	exit(0);
}

void clean(){
	
	CHECK_ERR(sem_close(oven_lock_sem));
	CHECK_ERR(sem_close(oven_size_sem));
	CHECK_ERR(sem_close(table_lock_sem));
	CHECK_ERR(sem_close(table_size_sem));
	CHECK_ERR(sem_close(table_empty_sem));
	
	CHECK_ERR(munmap(pizzeria, sizeof(restaurant)));
}

void setup(){

	srand(getpid()<<4);
	
	shared_mem = shm_open(SHARED_MEM, O_RDWR, 0666);
	CHECK_ERR(shared_mem);
	
	pizzeria =  mmap(NULL, sizeof(restaurant), PROT_WRITE, MAP_SHARED, shared_mem, 0);
	if (pizzeria == (void *) -1){
		printf("Error in file %s in line %d: %s\n", __FILE__, __LINE__, strerror(errno));
		exit(-1);
	}
	
	oven_lock_sem = sem_open(OVEN_LOCK_SEM,O_RDWR);
	validate_sem(oven_lock_sem);
	
	oven_size_sem = sem_open(OVEN_SIZE_SEM,O_RDWR);
	validate_sem(oven_size_sem);
	
	table_lock_sem = sem_open(TABLE_LOCK_SEM,O_RDWR);
	validate_sem(table_lock_sem);
	
	table_size_sem = sem_open(TABLE_SIZE_SEM,O_RDWR);
	validate_sem(table_size_sem);
	
	table_empty_sem = sem_open(TABLE_EMPTY_SEM,O_RDWR);
	validate_sem(table_empty_sem);
}

void make_pizza(int pizza){
	char time_str[30];
	timestamp(time_str);
	printf("(%d %s) Przygotowuję pizze: %d.\n", getpid(), time_str, pizza);
    usleep((rand() % 10 + 10) * 100000);
}

void put_to_oven(int pizza){

	CHECK_ERR(sem_wait(oven_size_sem));
	CHECK_ERR(sem_wait(oven_lock_sem));
	
	int free_id = (pizzeria -> oven_id + 1) % OVEN_SIZE;
	pizzeria->oven[free_id] = pizza;
	pizzeria->oven_size++;
	pizzeria->oven_id++;
	pizzeria->oven_id = pizzeria->oven_id % OVEN_SIZE;
	
	char time_str[30];
	timestamp(time_str);
	printf("(%d %s) Dodałem pizze: %d. Liczba pizz w piecu: %d.\n", getpid(), time_str, pizza, pizzeria->oven_size);
	
	CHECK_ERR(sem_post(oven_lock_sem));
	
}

int take_from_oven(){

	CHECK_ERR(sem_wait(oven_lock_sem));
	
	int pizza = pizzeria->oven[(pizzeria->oven_id - pizzeria->oven_size + 1 + OVEN_SIZE) % OVEN_SIZE];
	
	pizzeria->oven_size--;
	char time_str[30];
	timestamp(time_str);
	printf("(%d %s) [DODATKOWA INFORMACJA] Wyjmuję pizze: %d Na razie trzymam ją w rękach. Liczba pizz w piecu: %d.\n", getpid(), time_str, pizza, pizzeria->oven_size);
	
	CHECK_ERR(sem_post(oven_size_sem));
	CHECK_ERR(sem_post(oven_lock_sem));

	return pizza;
	
}

void put_on_table(int pizza){

	CHECK_ERR(sem_wait(table_size_sem));
	CHECK_ERR(sem_wait(table_lock_sem));
	CHECK_ERR(sem_post(table_empty_sem));
	
	int free_id = (pizzeria -> table_id + 1) % TABLE_SIZE;
	pizzeria->table[free_id] = pizza;
	pizzeria->table_size++;
	pizzeria->table_id++;
	pizzeria->table_id = pizzeria->table_id % TABLE_SIZE;
	
	char time_str[30];
	timestamp(time_str);
	printf("(%d %s) Wyjmuję pizze: %d. Liczba pizz w piecu: %d Liczba pizz na stole: %d.\n", getpid(), time_str, pizza, pizzeria->oven_size, pizzeria->table_size);
	
	CHECK_ERR(sem_post(table_lock_sem));
}

void work(){
	while (1){
		int pizza = rand() % 10;
		make_pizza(pizza);
		put_to_oven(pizza);
		usleep((rand() % (10) + 40) * 100000);
		pizza = take_from_oven();
		put_on_table(pizza);
	}
}


int main(){

	atexit(clean);
	signal(SIGINT, sigint_handler);
	setup();
	work();
}
