
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h> 
#include <sys/sem.h>
#include <sys/ipc.h>
#include "shared.h"

int shared_mem;
int semaphores;
int N;
int M;
pid_t* cooks;
pid_t* deliverers;

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short  *array;
	struct seminfo  *__buf;
};

void sigint_handler(int signum){
	exit(0);
}
     
void clean(){
	for (int i=0; i<N; i++){
		kill(cooks[i], SIGINT);
	}
	free(cooks);
	
	for (int i=0; i<M; i++){
		kill(deliverers[i], SIGINT);
	}
	free(deliverers);

	CHECK_ERR(semctl(semaphores, 0, IPC_RMID, NULL));
	CHECK_ERR(shmctl(shared_mem, IPC_RMID, NULL));
}

void setup(){
	// setup shared memory
	key_t mem_key = ftok("./main.c", MEM_ID);
	CHECK_ERR(mem_key);
	shared_mem = shmget(mem_key, sizeof(restaurant), IPC_CREAT | IPC_EXCL | 0666);
	CHECK_ERR(shared_mem);
	restaurant* pizzeria = shmat(shared_mem, NULL, 0);
	if (pizzeria == (void*)-1){
		printf("Error in file %s in line %d: %s\n", __FILE__, __LINE__, strerror(errno));
		exit(-1);
	}
	pizzeria->oven_id = -1;
	pizzeria->oven_size = 0;
	pizzeria->table_id = -1;
	pizzeria->table_size = 0;

	CHECK_ERR(shmdt(pizzeria));
	
	//setup semaphores
	key_t sem_key = ftok("./main.c", SEM_ID);
	CHECK_ERR(sem_key);
	semaphores = semget(sem_key, 5,  IPC_CREAT | IPC_EXCL | 0666);
	CHECK_ERR(semaphores);
	
	union semun sem_arg;
	
	sem_arg.val = OVEN_SIZE;
	CHECK_ERR(semctl(semaphores, OVEN_SIZE_SEM, SETVAL, sem_arg));
	
	sem_arg.val = TABLE_SIZE;
	CHECK_ERR(semctl(semaphores, TABLE_SIZE_SEM, SETVAL, sem_arg));
	
	sem_arg.val = 1;
	CHECK_ERR(semctl(semaphores, OVEN_LOCK_SEM, SETVAL, sem_arg));
	CHECK_ERR(semctl(semaphores, TABLE_LOCK_SEM, SETVAL, sem_arg));
	
	sem_arg.val = 0;
	CHECK_ERR(semctl(semaphores, TABLE_EMPTY_SEM, SETVAL, sem_arg));
	
}

void spawn_worksers(){
	cooks = calloc(N, sizeof(pid_t));
	for (int i = 0; i < N; i++){
		pid_t child = fork();
		CHECK_ERR(child);
		if (child == 0){
			execlp("./cook", "cook", NULL);
		}
		cooks[i] = child;
	}
	
	deliverers = calloc(M, sizeof(pid_t));
	for (int i = 0; i < M; i++){
		pid_t child = fork();
		CHECK_ERR(child);
		if (child == 0){
			execlp("./deliverer", "deliverer", NULL);
		}
		deliverers[i] = child;
	}

}

int main(int argc, char **argv) {
	if (argc !=3){
		fprintf(stderr, "Invalid number of arguments\n");
		exit(-1);
	}
	
	N = atoi(argv[1]);
	M = atoi(argv[2]);
	
	atexit(clean);
	signal(SIGINT, sigint_handler);
	setup();
	spawn_worksers();
	
	while(wait(NULL)!= -1){}

}

