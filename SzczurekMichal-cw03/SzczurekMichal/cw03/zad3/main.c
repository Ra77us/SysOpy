#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <dirent.h>
#include <string.h>

int is_valid_file(char* name){

    char supported_ext [5][6] = {".txt", ".doc", ".docx", ".json", ".tex"};
    for (int i=0; i<5; i++){
        if (strlen(supported_ext[i]) < strlen(name)){
            int j = strlen(supported_ext[i])-1;
            int k = strlen(name)-1;
            while(j >= 0 && name[k] == supported_ext[i][j]){
                j--;
                k--;
            }
            if (j<0)
                return 0;
        }
    }
    return -1;
}

int contains(char* file_name, char* pattern){

    FILE* file = fopen(file_name,"r");
    if (file == NULL){
        fprintf(stderr, "Cannot open file %s by%d\n", file_name, getpid());
        return -1; // do not end process, assume that file does not contain pattern
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = calloc(size +1, sizeof(char));
    fread(buffer, 1, size, file);
    fclose(file);

    int res = -1;
    if (strstr(buffer, pattern)){
        res = 0;
    }
    free(buffer);
    return res;

}


// The search is implemented is such way that it first looks for files
// and only then goes to directories
// This require to to look over directory contents twice
// However the output is more readable as files in same directory are listed next to each other

void search( char* path, char* pattern, int depth_left){
    if (depth_left < 0)
        return;
    printf("\n=== Now searching in %s by process %d created by %d ===\n",path, getpid(),getppid());
    depth_left--;
    DIR* dir = opendir(path);
    if (dir == NULL){
        fprintf(stderr, "Cannot open directory %s\n", path);
        exit(1);
    }

    struct dirent* curr;
    int found = 0;

    while ((curr = readdir(dir)) != NULL){
        char* curr_path = calloc(strlen(path) + strlen(curr->d_name) + 2, sizeof(char));
        sprintf(curr_path, "%s/%s", path, curr->d_name);
        if (curr->d_type == DT_REG){
            if (is_valid_file(curr_path) == 0){
                if (contains(curr_path, pattern) == 0){
                    printf("Found matching pattern in %s [PATH: %s PID: %d]\n", curr->d_name, curr_path, getpid());
                    found ++;
                }
            }
        }
        free(curr_path);

    }
    if (found == 0){
        printf("Nothing found (searched by process %d)\n", getpid());
    }

    rewinddir(dir);

    while ((curr = readdir(dir)) != NULL){

        char* curr_path = calloc(strlen(path) + strlen(curr->d_name) + 2, sizeof(char));
        sprintf(curr_path, "%s/%s", path, curr->d_name);
        if ((strcmp(curr->d_name, ".") != 0) && (strcmp(curr->d_name, "..") != 0) && curr->d_type == DT_DIR){
            if(fork()==0){
                search(curr_path, pattern, depth_left);
                free(curr_path);
                exit(0);
            }
        }
        free(curr_path);
        while(wait(NULL) != -1){}


    }
    closedir(dir);

}


int main(int argc,char* argv[]){
    if (argc != 4){
        fprintf(stderr, "Invalid number of arguments\n");
        exit(1);
    }

    int max_depth = atoi(argv[3]);

    if (max_depth < 0){
        fprintf(stderr, "Max depth cannot be negative value\n");
        exit(1);
    }

    search(argv[1], argv[2], max_depth);
    while(wait(NULL) != -1){}

}
