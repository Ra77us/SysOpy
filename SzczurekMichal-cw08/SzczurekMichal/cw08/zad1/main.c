#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>

// makro sprawdzające wystąpienie błędu (zwrócenie -1)
#define CHECK_ERR(f) do {if ((f) == -1) {printf("Error in file %s in line %d: %s\n", __FILE__, __LINE__, strerror(errno)); exit(-1);}} while(0)


typedef struct interval{
	int start;
	int stop;
}interval;


int** picture;
int** res;
int width;
int height;
int n;
int* values_count = NULL;
interval* intervals = NULL;

int calculate_time(struct timeval start){
	struct timeval end;
   	gettimeofday(&end,NULL);
    return (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);

}

void load_picture(char* in_file, char* variant){

	FILE* file = fopen(in_file, "r");
	if(file == NULL){
		printf("Error in file %s in line %d: %s\n", __FILE__, __LINE__, strerror(errno)); 
		exit(-1);
	}
	
	int flag = 0;
	if (strcmp(variant, "numbers") == 0){
		flag = 1;
		values_count = calloc(256, sizeof(int));
		for (int i = 0; i < 256; i++){
			values_count[i] = 0;
		}
	}
	
	char* line;
	size_t size = 0;
	getline(&line, &size, file);
	getline(&line, &size, file);
	getline(&line, &size, file);
	
	width = atoi(strtok(line, " \t\n"));
    height = atoi(strtok(NULL, " \t\n"));
    
	picture = calloc(width, sizeof(int*));
	
	for (int i = 0; i < width; i++){
		picture[i] = calloc(height, sizeof(int));
	}
	
	getline(&line, &size, file);

	for (int i = 0; i < height; i++){
		for (int j = 0; j < width; j++){
			fscanf(file, "%d", &picture[j][i]);
			if (flag == 1)
				values_count[picture[j][i]]++;
			}
	}
	fclose(file);
	free(line);
}

void save_picture(char* save_file){
	
	FILE* file = fopen(save_file, "w");
	if(file == NULL){
		printf("Error in file %s in line %d: %s\n", __FILE__, __LINE__, strerror(errno)); 
		exit(-1);
	}
	
	fprintf(file, "P2\n");
    fprintf(file, "# Processed picture\n");
    fprintf(file, "%d  %d\n", width, height);
    fprintf(file, "255\n");
    
    int len = 0;
    
    for (int i = 0; i < height; i++){
		for (int j = 0; j < width; j++){
			char val[10];
			sprintf(val, "%d", res[j][i]);
			if (len + strlen(val) + 2 > 70){
				fprintf(file, "\n");
				len = 0;
			}
			len += strlen(val) + 1;
			fprintf(file, "%s ", val);
		}
	}	
	fclose(file);
}

void* block_version(void* arg){
	struct timeval start;
   	gettimeofday(&start,NULL);
	int* k = arg;
	int min_x = (*k-1) * ceil((double) width / n);
	int max_x = *k * ceil((double) width / n) - 1;
	if (max_x > width-1)
		max_x = width-1;
		
	for (int i = min_x; i <= max_x; i++){
		for (int j = 0; j < height; j++){
			res[i][j] = 255 - picture[i][j];
			}
	}
	
	int* res = calloc(1, sizeof(double));
	*res = calculate_time(start); 
	return (void*) res;
}

void calculate_intervals(){
	intervals = calloc(n, sizeof(interval));
	int j = 0;
	int sum = 0;
	intervals[0].start = 0;
	for (int i = 0; i<n-1; i++){
		sum = 0;
		while(sum < width * height / n && j < 256){
			sum += values_count[j];
			j++;
		}
		intervals[i].stop = j;
		intervals[i+1].start = j+1;
		if (intervals[i+1].stop > 255)
			intervals[i+1].stop = 255;
	
	}
	intervals[n-1].stop = 255;
}

void* numbers_version(void* arg){
	struct timeval start;
   	gettimeofday(&start,NULL);
	int* k = arg;
	for (int i = 0; i < width; i++){
		for (int j = 0; j < height; j++){
			if (picture[i][j] >= intervals[*k].start && picture[i][j] <= intervals[*k].stop){
				res[i][j] = 255 - picture[i][j];
				}
		}
	}
	
	int* res = calloc(1, sizeof(double));
	*res = calculate_time(start); 
	return (void*) res;
}

int main(int argc, char** argv){

	if (argc !=5){
		fprintf(stderr, "Invalid number of arguments\n");
		exit(-1);
	}
	
	if (!(strcmp(argv[2], "block")==0) && !(strcmp(argv[2], "numbers")==0))
	{
		fprintf(stderr, "Invalid variant\n");
		exit(-1);
	}
	
	n = atoi(argv[1]);
	
	load_picture(argv[3], argv[2]);
	
	res = calloc(width, sizeof(int*));
	for (int i = 0; i < width; i++){
		res[i] = calloc(height, sizeof(int));
	}
	
	struct timeval start;
   	gettimeofday(&start,NULL);
   	
	pthread_t *threads = calloc(n, sizeof(pthread_t));
	int* arg_vec = calloc(n, sizeof(int));

	if (strcmp(argv[2], "block")==0){
		for (int i = 0; i < n; i++) { 
			arg_vec[i] = i+1;
		    CHECK_ERR(pthread_create(&threads[i], NULL,  &block_version, &arg_vec[i]));
		}
	}
	
	else{ //numbers
		calculate_intervals();
		for (int i = 0; i < n; i++) { 
			arg_vec[i] = i;
		    CHECK_ERR(pthread_create(&threads[i], NULL,  &numbers_version, &arg_vec[i]));
		}
	}
	
	int* times = calloc(n, sizeof(double));
	for (int i = 0; i < n; i++) {
    	int* res;
        CHECK_ERR(pthread_join(threads[i], (void **) &res));
        times[i] = *res;
        free(res);
    }
    
    int total_time = calculate_time(start);
    
    save_picture(argv[4]);
    
    for (int i = 0; i < n; i++) {
		printf("It took %d microseconds for thred %d to finish\n", times[i], i+1); 
		if (strcmp(argv[2], "block")!=0){
        	int total = 0;
        	for (int j=intervals[i].start; j<= intervals[i].stop; j++)
        		total += values_count[j];
        	printf("This thread has processed pixels in range %d to %d. That is %d pixels in total\n", intervals[i].start, intervals[i].stop, total);
    	}
    }
    printf("\nTotal time: %d microseconds\n", total_time);	
    
	free(times);
	for (int i = 0; i < width; i++){
		free(picture[i]);
	}
	free(picture);
	for (int i = 0; i < width; i++){
		free(res[i]);
	}
	free(res);
	free(threads);
	free(arg_vec);
	if (values_count != NULL)
		free(values_count);
	if (intervals != NULL)
		free(intervals);
}
