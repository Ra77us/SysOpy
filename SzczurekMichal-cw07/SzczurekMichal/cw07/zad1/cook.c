
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h> 
#include <sys/ipc.h>
#include "shared.h"
#include <sys/sem.h>

int semaphores;
int shared_mem;
restaurant* pizzeria;


void clean(){
	CHECK_ERR(shmdt(pizzeria));
}

void sigint_handler(int signum){
	exit(0);
}

void setup(){

	srand(getpid()<<4);
	
	key_t sem_key = ftok("./main.c", SEM_ID);
	CHECK_ERR(sem_key);
	semaphores = semget(sem_key, 4, 0);
	CHECK_ERR(semaphores);
	
	key_t mem_key = ftok("./main.c", MEM_ID);
	CHECK_ERR(mem_key);
	shared_mem = shmget(mem_key, sizeof(restaurant), 0);
	CHECK_ERR(shared_mem);
	
	pizzeria = shmat(shared_mem, NULL, 0);
	if (pizzeria == (void*)-1){
		printf("Error in file %s in line %d: %s\n", __FILE__, __LINE__, strerror(errno));
		exit(-1);
	}
}

void make_pizza(int pizza){
	char time_str[30];
	timestamp(time_str);
	printf("(%d %s) Przygotowuję pizze: %d\n.", getpid(), time_str, pizza);
    usleep((rand() % 10 + 10) * 100000);
}

void put_to_oven(int pizza){
	struct sembuf lock_oven = {OVEN_LOCK_SEM, -1, 0};
	struct sembuf increase_size_oven = {OVEN_SIZE_SEM, -1, 0};
	struct sembuf operations[2] = {lock_oven, increase_size_oven};
	CHECK_ERR(semop(semaphores, operations, 2));
	
	int free_id = (pizzeria -> oven_id + 1) % OVEN_SIZE;
	pizzeria->oven[free_id] = pizza;
	pizzeria->oven_size++;
	pizzeria->oven_id++;
	pizzeria->oven_id = pizzeria->oven_id % OVEN_SIZE;
	
	char time_str[30];
	timestamp(time_str);
	printf("(%d %s) Dodałem pizze: %d. Liczba pizz w piecu: %d.\n", getpid(), time_str, pizza, pizzeria->oven_size);
	
	struct sembuf unlock_oven = {OVEN_LOCK_SEM, 1, 0};
	CHECK_ERR(semop(semaphores, &unlock_oven, 1));
	
}

int take_from_oven(){

	struct sembuf lock_oven = {OVEN_LOCK_SEM, -1, 0};
	CHECK_ERR(semop(semaphores, &lock_oven, 1));
	
	int pizza = pizzeria->oven[(pizzeria->oven_id - pizzeria->oven_size + 1 + OVEN_SIZE) % OVEN_SIZE];
	pizzeria->oven_size--;
	
	char time_str[30];
	timestamp(time_str);
	printf("(%d %s) [DODATKOWA INFORMACJA] Wyjmuję pizze: %d Na razie trzymam ją w rękach. Liczba pizz w piecu: %d.\n", getpid(), time_str, pizza, pizzeria->oven_size);
	
	struct sembuf unlock_oven = {OVEN_LOCK_SEM, 1, 0};
	struct sembuf decrease_size_oven = {OVEN_SIZE_SEM, 1, 0};
	struct sembuf operations[2] = {unlock_oven, decrease_size_oven};
	CHECK_ERR(semop(semaphores, operations, 2));
	
	return pizza;
	
}

void put_on_table(int pizza){

	struct sembuf lock_table = {TABLE_LOCK_SEM, -1, 0};
	struct sembuf increase_size_table = {TABLE_SIZE_SEM, -1, 0};
	struct sembuf add_pizza_table = {TABLE_EMPTY_SEM, 1, 0};
	struct sembuf operations[3] = {lock_table, increase_size_table, add_pizza_table};
	CHECK_ERR(semop(semaphores, operations, 3));
	
	int free_id = (pizzeria -> table_id + 1) % TABLE_SIZE;
	pizzeria->table[free_id] = pizza;
	pizzeria->table_size++;
	pizzeria->table_id++;
	pizzeria->table_id = pizzeria->table_id % TABLE_SIZE;
	
	char time_str[30];
	timestamp(time_str);
	printf("(%d %s) Wyjmuję pizze: %d. Liczba pizz w piecu: %d Liczba pizz na stole: %d.\n", getpid(), time_str, pizza, pizzeria->oven_size, pizzeria->table_size);
	
	struct sembuf unlock_table = {TABLE_LOCK_SEM, 1, 0};
	CHECK_ERR(semop(semaphores, &unlock_table, 1));
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
