#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


#define ELVES_NO 10
#define REINDEERS_NO 9
#define DELIVERIES_NO 3
#define REQUIRED_ELVES_NO 3

pthread_mutex_t sleep_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t reindeers_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t elves_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t elves_waiting_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t reindeers_waiting_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t delivery_ended_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t santa_sleeping_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t santa_task_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t santa_helping_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t santa_helped_cond = PTHREAD_COND_INITIALIZER;

int waiting_reindeers = 0;
int waiting_elves = 0;
int deliveries = 0;
int santa_sleeping = 1;
int solving_time = 0;
int *reindeer_ids;
int *elf_ids;
int elves_queue[REQUIRED_ELVES_NO];
pthread_t elf_threads[ELVES_NO];
pthread_t reindeer_threads[REINDEERS_NO];
pthread_t santa_thread;
int completed_elves[ELVES_NO] = {0};

void santa_func() {
    srand(pthread_self());

    while (1) {

        pthread_mutex_lock(&sleep_mutex);
        pthread_cond_wait(&santa_task_cond, &sleep_mutex);
        santa_sleeping = 0;
        pthread_mutex_unlock(&sleep_mutex);

        printf("Mikolaj: Budze sie\n");

        pthread_mutex_lock(&reindeers_mutex);
        if (waiting_reindeers == REINDEERS_NO) {
            printf("Mikolaj: Dostarczam zabawki po raz %d\n", deliveries + 1);
            usleep((rand() % (2001) + 2000) * 1000);
            deliveries++;
            pthread_cond_broadcast(&delivery_ended_cond);
            waiting_reindeers = 0;
            pthread_mutex_unlock(&reindeers_mutex);
            if (deliveries == DELIVERIES_NO) {
                printf("Mikolaj: Rozwiozlem juz wszystko\n");
                pthread_exit((void *) 0);
            }
        } else {
            pthread_mutex_unlock(&reindeers_mutex);

            printf("Mikolaj: Rozwiazuje problemy elfow %d %d %d\n", elves_queue[0], elves_queue[1], elves_queue[2]);
            solving_time = (rand() % (1001) + 1000) * 1000;
            pthread_mutex_lock(&elves_mutex);
            pthread_cond_broadcast(&santa_helping_cond);
            pthread_mutex_unlock(&elves_mutex);
            usleep(solving_time);
            printf("Mikolaj: Rozwiazalem problemy elfow %d %d %d\n", elves_queue[0], elves_queue[1], elves_queue[2]);
            pthread_mutex_lock(&elves_mutex);
            pthread_cond_broadcast(&santa_helped_cond);
            waiting_elves = 0;
            pthread_mutex_unlock(&elves_mutex);
        }

        pthread_mutex_lock(&sleep_mutex);
        santa_sleeping = 1;
        printf("Mikolaj: Zasypiam\n");
        pthread_cond_broadcast(&santa_sleeping_cond);
        pthread_mutex_unlock(&sleep_mutex);

    }
}

void reindeer_func(int *id) {
    srand((*id + getpid()) << 16);
    while (deliveries < DELIVERIES_NO) {
        usleep((rand() % (5001) + 5000) * 1000);

        pthread_mutex_lock(&reindeers_mutex);
        waiting_reindeers++;
        printf("Renifer [ID %d]: Czekam na Mikolaja. Obecnie czeka %d reniferow\n", *id, waiting_reindeers);
        if (waiting_reindeers == REINDEERS_NO) {
            pthread_mutex_unlock(&reindeers_mutex);
            pthread_mutex_lock(&sleep_mutex);
            while (santa_sleeping == 0) {
                pthread_cond_wait(&santa_sleeping_cond, &sleep_mutex);
            }
            pthread_mutex_unlock(&sleep_mutex);

            printf("Renifer [ID %d]: Wybudzam Mikolaja\n", *id);
            pthread_cond_broadcast(&santa_task_cond);
        } else {
            pthread_mutex_unlock(&reindeers_mutex);
        }
        pthread_mutex_lock(&reindeers_waiting_mutex);
        pthread_cond_wait(&delivery_ended_cond, &reindeers_waiting_mutex);
        pthread_mutex_unlock(&reindeers_waiting_mutex);
    }
    pthread_exit((void *) 0);
}

void cleanup_elf(void *arg) {
    int *id = arg;
    pthread_mutex_unlock(&elves_waiting_mutex);
    pthread_mutex_unlock(&sleep_mutex);
    pthread_mutex_unlock(&elves_mutex);
    completed_elves[*id - 1] = 1;
}

void elf_func(int *id) {
    srand((*id + getpid()) << 16);
    pthread_cleanup_push(cleanup_elf, id);
    while (1) {
        int first_wait = 1;
        usleep((rand() % (3001) + 2000) * 1000);
        pthread_mutex_lock(&elves_waiting_mutex);
        while (waiting_elves == REQUIRED_ELVES_NO) {
            if (first_wait) {
                printf("Elf [ID %d]: Czekam na powrot elfow\n", *id);
                first_wait = 0;
            }
          
            pthread_cond_wait(&santa_helped_cond, &elves_waiting_mutex);
        }
        first_wait = 1;
        pthread_mutex_lock(&elves_mutex);
        waiting_elves++;
        pthread_mutex_unlock(&elves_waiting_mutex);

        elves_queue[waiting_elves-1] = *id;
      
        printf("Elf [ID %d]: Czekam na Mikolaja. Obecnie czekaja %d elfy\n", *id, waiting_elves);
        if (waiting_elves == REQUIRED_ELVES_NO) {
            pthread_mutex_unlock(&elves_mutex);
            pthread_mutex_lock(&sleep_mutex);
            pthread_mutex_lock(&reindeers_mutex);
            while (santa_sleeping == 0 || waiting_reindeers == 9) {
                pthread_mutex_unlock(&reindeers_mutex);
                pthread_cond_wait(&santa_sleeping_cond, &sleep_mutex);
                pthread_mutex_lock(&reindeers_mutex);
            }
	    	pthread_mutex_unlock(&reindeers_mutex);
            printf("Elf [id %d]: Wybudzam Mikolaja\n", *id);
            pthread_mutex_lock(&elves_mutex);
            pthread_cond_broadcast(&santa_task_cond);
            pthread_mutex_unlock(&sleep_mutex);
            
           
        }
        pthread_cond_wait(&santa_helping_cond, &elves_mutex);
        pthread_mutex_unlock(&elves_mutex);
        printf("Elf [id %d]: Mikolaj rozwiazuje moje problemy\n", *id);
        usleep(solving_time);
    }
    pthread_cleanup_pop(1);
}

void clean() {

    pthread_join(santa_thread, NULL);

    for (int i = 0; i < REINDEERS_NO; i++) {
        pthread_join(reindeer_threads[i], NULL);
    }

    for (int i = 0; i < ELVES_NO; i++) {
        pthread_cancel(elf_threads[i]);
    }

 

    while (1) { // wait for elves to finish and close resources
        int not_done = 0;
        for (int i = 0; i < ELVES_NO; i++) {
            if (completed_elves[i] != 1)
                not_done = 1;
        }
        if (not_done == 0) {
           	pthread_mutex_destroy(&reindeers_mutex);
			pthread_mutex_destroy(&elves_mutex);
			pthread_mutex_destroy(&sleep_mutex);
			pthread_mutex_destroy(&elves_waiting_mutex);
			pthread_mutex_destroy(&reindeers_waiting_mutex);

			pthread_cond_destroy(&delivery_ended_cond);
			pthread_cond_destroy(&santa_sleeping_cond);
			pthread_cond_destroy(&santa_task_cond);
			pthread_cond_destroy(&santa_helping_cond);
    
            free(elf_ids);
            free(reindeer_ids);
            exit(0);
        }
    }

}

void create_threads() {

    pthread_create(&santa_thread, NULL, (void *(*)(void *)) santa_func, NULL);

    reindeer_ids = calloc(REINDEERS_NO, sizeof(int));
    for (int i = 0; i < REINDEERS_NO; i++) {
        reindeer_ids[i] = i + 1;
        pthread_create(&reindeer_threads[i], NULL, (void *(*)(void *)) reindeer_func, &reindeer_ids[i]);
    }


    elf_ids = calloc(ELVES_NO, sizeof(int));
    for (int i = 0; i < ELVES_NO; i++) {
        elf_ids[i] = i + 1;
        pthread_create(&elf_threads[i], NULL, (void *(*)(void *)) elf_func, &elf_ids[i]);
    }
}

int main() {

    create_threads();
    clean();

}
