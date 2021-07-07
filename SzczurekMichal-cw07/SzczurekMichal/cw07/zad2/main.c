
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/time.h>
#include "shared.h"


int N;
int M;
pid_t* cooks;
pid_t* deliverers;

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

	CHECK_ERR(shm_unlink(SHARED_MEM));
	
	CHECK_ERR(sem_unlink(OVEN_LOCK_SEM));
	CHECK_ERR(sem_unlink(OVEN_SIZE_SEM));
	CHECK_ERR(sem_unlink(TABLE_LOCK_SEM));
	CHECK_ERR(sem_unlink(TABLE_SIZE_SEM));
	CHECK_ERR(sem_unlink(TABLE_EMPTY_SEM));
}

void setup(){

	// setup shared memory
	int shared_mem = shm_open(SHARED_MEM, O_CREAT | O_RDWR, 0666);
	CHECK_ERR(ftruncate(shared_mem, sizeof(restaurant)));
	CHECK_ERR(shared_mem);
	restaurant* pizzeria =  mmap(NULL, sizeof(restaurant), PROT_WRITE, MAP_SHARED, shared_mem, 0);
	if (pizzeria == (void *) -1){
		printf("Error in file %s in line %d: %s\n", __FILE__, __LINE__, strerror(errno));
		exit(-1);
	}
	
	pizzeria->oven_id = -1;
	pizzeria->oven_size = 0;
	pizzeria->table_id = -1;
	pizzeria->table_size = 0;

	CHECK_ERR(munmap(pizzeria, sizeof(restaurant)));
	
	//setup semaphores
	sem_t *sem = sem_open(OVEN_LOCK_SEM, O_CREAT | O_RDWR, 0666, 1);
	validate_sem(sem);
	CHECK_ERR(sem_close(sem));
	
	sem = sem_open(OVEN_SIZE_SEM, O_CREAT | O_RDWR, 0666, OVEN_SIZE);
	validate_sem(sem);
	CHECK_ERR(sem_close(sem));
	
	sem = sem_open(TABLE_LOCK_SEM, O_CREAT | O_RDWR, 0666, 1);
	validate_sem(sem);
	CHECK_ERR(sem_close(sem));
	
	sem = sem_open(TABLE_SIZE_SEM, O_CREAT | O_RDWR, 0666, TABLE_SIZE);
	validate_sem(sem);
	CHECK_ERR(sem_close(sem));
	
	sem = sem_open(TABLE_EMPTY_SEM, O_CREAT | O_RDWR, 0666, 0);
	validate_sem(sem);
	CHECK_ERR(sem_close(sem));
	 
	
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

