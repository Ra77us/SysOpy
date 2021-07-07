#define  _GNU_SOURCE

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "lib1.h"
#include <string.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>

#ifdef DLL
#include <dlfcn.h>
#endif

typedef struct time_wrapper{
    clock_t start_real;
    clock_t start_user;
    clock_t start_system;
}wrapper;

int is_positive_num(char *arg) {
    for (int i = 0; i < strlen(arg); i++) {
        if (!isdigit(arg[i])) {
            return 0;
        }
    }
    return 1;
}

int split_file_arg(char *arg, char **f1, char **f2) {
    int split_point = -1;
    for (int i = 0; i < strlen(arg); i++) {
        if (arg[i] == ':') {
            // there are at least two ':', such arg is invalid
            if (split_point != -1) {
                return 0;
            }
            split_point = i;
        }
    }
    if (split_point == -1 || split_point == 0 || split_point == strlen(arg) - 1)
        return 0;

    *f1 = (char *) calloc((split_point + 1), sizeof(char));
    strncpy(*f1, arg, split_point);

    *f2 = (char *) calloc(strlen(arg) - split_point, sizeof(char));
    strncpy(*f2, arg + split_point + 1, strlen(arg) - split_point - 1);

    return 1;
}

void set_times(wrapper* time_wrapper) {
    struct tms time_res;
    time_wrapper->start_real = times(&time_res);
    time_wrapper->start_user = time_res.tms_utime;
    time_wrapper->start_system = time_res.tms_stime;
}

void add_times(wrapper* time_wrapper, double *real_time, double *user_time, double *system_time) {
    struct tms time_res;
    clock_t curr;
    curr = times(&time_res);

    *real_time += (double) (curr - time_wrapper->start_real) / sysconf(_SC_CLK_TCK);
    *user_time += (double) (time_res.tms_utime - time_wrapper->start_user) / sysconf(_SC_CLK_TCK);
    *system_time += (double) (time_res.tms_stime - time_wrapper->start_system) / sysconf(_SC_CLK_TCK);
}

void write_res(FILE *res, double real_time, double user_time, double system_time, char *operation) {

    printf("\nTotal time of %s:\n", operation);
    printf("Real time: %lf s.\n", real_time);
    printf("User time: %lf s.\n", user_time);
    printf("System time: %lf s.\n", system_time);

    fprintf(res, "\nTotal time of %s:\n", operation);
    fprintf(res, "Real time: %lf s.\n", real_time);
    fprintf(res, "User time: %lf s.\n", user_time);
    fprintf(res, "System time: %lf s.\n", system_time);
}

void test(int n, int consequent_n, int consequent_repetitions, char *files, char *res) {

#ifdef DLL
    void *handle = dlopen("./liblib1.so", RTLD_LAZY);
    if(!handle){
        printf("Cannot open library");
        exit(1);
    }
    main_array* (*new_main_array)(int) = dlsym(handle, "new_main_array");
    file_pair* (*new_file_pair)(char*, char* ) = dlsym(handle, "new_file_pair");
    int (*insert_row_block)(FILE*, main_array*) = dlsym(handle, "insert_row_block");
    FILE* (*merge)(file_pair*) = dlsym(handle, "merge");
    void (*remove_block)(main_array*, int) = dlsym(handle, "remove_block");
    void (*remove_main_array)(main_array*) = dlsym(handle, "remove_main_array");
#endif

    double real_time[4];
    double user_time[4];
    double system_time[4];
    wrapper time_wrapper[4];
    char operation_names[4][40] = {"merging", "allocating blocks", "removing blocks", "consequent adding and removing"};
    for (int i = 0; i < 4; i++) {
        real_time[i] = 0;
        user_time[i] = 0;
        system_time[i] = 0;
    }

    FILE *res_f = fopen(res, "a");
    if (res_f == NULL) {
        printf("Cannot open result file.\n");
        exit(1);
    }

    main_array *table = new_main_array(n + consequent_n * consequent_repetitions);
    char *f1;
    char *f2;
    if (split_file_arg(files, &f1, &f2) != 1) {
        printf("Cannot open files\n");
        exit(1);
    }
    file_pair *pair = new_file_pair(f1, f2);
    fprintf(res_f, "\n ===== TEST FOR %s AND %s. =====\n", f1, f2);
    fprintf(res_f, "TEST CONSIST OF %d STANDARD OPERATIONS AND %d CONSEQUENT OPERATIONS CONSISTING OF %d FILES\n", n,
            consequent_repetitions, consequent_n);

    for (int i = 0; i < n; i++) {
        set_times(&time_wrapper[0]);
        FILE *tmp = merge(pair);
        add_times(&time_wrapper[0], &real_time[0], &user_time[0], &system_time[0]);

        set_times(&time_wrapper[1]);
        insert_row_block(tmp, table);
        add_times(&time_wrapper[1], &real_time[1], &user_time[1], &system_time[1]);

        fclose(tmp);
    }

    set_times(&time_wrapper[2]);
    for (int i = 0; i < n; i++) {
        remove_block(table, i);
    }
    add_times(&time_wrapper[2], &real_time[2], &user_time[2], &system_time[2]);

    set_times(&time_wrapper[3]);
    for (int i = 0; i < consequent_repetitions; i++) {
        for (int j = 0; j < consequent_n; j++) {
            FILE *tmp = merge(pair);
            insert_row_block(tmp, table);
            fclose(tmp);
        }
        for (int j = 0; j < consequent_n; j++) {
            remove_block(table, j + n + i * consequent_n);
        }
    }
    add_times(&time_wrapper[3], &real_time[3], &user_time[3], &system_time[3]);

    for (int i = 0; i < 4; i++) {
        write_res(res_f, real_time[i], user_time[i], system_time[i], operation_names[i]);
    }

    free(f1);
    free(f2);
    free(pair);
    remove_main_array(table);
    fclose(res_f);

#ifdef DLL
    dlclose(handle);
#endif
}


int main(int argc, char **argv) {
    main_array *table = NULL;
#ifdef DLL
    void *handle = dlopen("./liblib1.so", RTLD_LAZY);
    if(!handle){
        printf("Cannot open library");
        exit(1);
    }

    main_array* (*new_main_array)(int) = dlsym(handle, "new_main_array");
    file_pair* (*new_file_pair)(char*, char* ) = dlsym(handle, "new_file_pair");
    int (*insert_row_block)(FILE*, main_array*) = dlsym(handle, "insert_row_block");
    FILE* (*merge)(file_pair*) = dlsym(handle, "merge");
    void (*print_main_array)(main_array*) = dlsym(handle, "print_main_array");
    void (*remove_row)(row_block*, int) = dlsym(handle, "remove_row");
    void (*remove_block)(main_array*, int) = dlsym(handle, "remove_block");
    void (*remove_main_array)(main_array*) = dlsym(handle, "remove_main_array");
    void (*print_row_block)(row_block*) = dlsym(handle, "print_row_block");
    int (*get_size)( row_block*) = dlsym(handle, "get_size");
#endif

    for (int i = 1; i < argc; i++) {

        //CREATE TABLE
        if (strcmp(argv[i], "create_table") == 0) {
            if (argc > i + 1 && is_positive_num(argv[i + 1]) == 1) {
                int size = atoi(argv[i + 1]);
                if (table != NULL)
                    remove_main_array(table);
                table = new_main_array(size);
                i++;
            } else {
                printf("Invalid argument after \"create_table\".\n");
                exit(1);
            }
        }
            //MERGE
        else if (strcmp(argv[i], "merge_files") == 0) {
            char *f1;
            char *f2;
            int done = 0;
            while (i + 1 < argc) {
                //invalid arg
                if (split_file_arg(argv[i + 1], &f1, &f2) != 1) {
                    break;
                }
                if (table == NULL) {
                    printf("No table created. Merging failed.\n");
                    exit(1);
                }
                if (table->curr_id >= table->initial_size) {
                    printf("Not enough space in table for new block.\n");
                    exit(1);
                }

                file_pair *pair = new_file_pair(f1, f2);
                FILE *tmp = merge(pair);
                insert_row_block(tmp, table);

                fclose(tmp);
                free(f1);
                free(f2);
                free(pair);
                done += 1;
                i += 1;
            }
            // no files merged
            if (done == 0) {
                printf("Invalid arguments after merge_files.\n");
                exit(1);
            }
        }

            // REMOVE BLOCK
        else if (strcmp(argv[i], "remove_block") == 0) {
            if (i + 1 < argc && is_positive_num(argv[i + 1])) {
                remove_block(table, atoi(argv[i + 1]));
                i += 1;
            } else {
                printf("Invalid argument after remove_block.\n");
                exit(1);
            }

        }
            // REMOVE ROW
        else if (strcmp(argv[i], "remove_row") == 0) {
            if (i + 2 < argc && is_positive_num(argv[i + 1]) && is_positive_num(argv[i + 2])) {
                if (atoi(argv[i + 1]) < table->curr_id && table->arr[atoi(argv[i + 1])]->size != 0) {
                    remove_row(table->arr[atoi(argv[i + 1])], atoi(argv[i + 2]));
                    i += 2;
                } else {
                    printf("Block ID out of range.\n");
                    exit(1);
                }

            } else {
                printf("Invalid arguments after remove_row.\n");
                exit(1);
            }
        }

            // GET_BLOCK_SIZE
        else if (strcmp(argv[i], "get_block_size") == 0) {
            if (i + 1 < argc && is_positive_num(argv[i + 1]) && atoi(argv[i + 1]) < table->curr_id &&
                table->arr[atoi(argv[i + 1])] != NULL) {
                printf("Block of size: %d\n", get_size(table->arr[atoi(argv[i + 1])]));
                i += 1;
            } else {
                printf("Invalid argument after get_block_size.\n");
                exit(1);
            }

        }
            // PRINT ROW BLOCK
        else if (strcmp(argv[i], "print_row_block") == 0) {
            if (i + 1 < argc && is_positive_num(argv[i + 1]) && atoi(argv[i + 1]) < table->curr_id &&
                table->arr[atoi(argv[i + 1])] != NULL) {
                print_row_block(table->arr[atoi(argv[i + 1])]);
                i += 1;
            } else {
                printf("Invalid block to print.\n");
                exit(1);
            }

        }

            // PRINT MAIN ARRAY
        else if (strcmp(argv[i], "print_main_array") == 0) {
            if (table != NULL) {
                print_main_array(table);
                i += 1;
            } else {
                printf("No array to print.\n");
                exit(1);
            }

        }
            // SETUP TEST
        else if (strcmp(argv[i], "test") == 0) {
            if (argc < i + 5) {
                printf("Test has to have 3 number arguments, files to operate and result file.\n");
                exit(1);
            }

            if (!is_positive_num(argv[i + 1]) || !is_positive_num(argv[i + 2]) || !is_positive_num(argv[i + 3])) {
                printf("Test numbers has to be positive numbers.\n");
                exit(1);
            }

            test(atoi(argv[i + 1]), atoi(argv[i + 2]), atoi(argv[i + 3]), argv[i + 4], argv[i + 5]);

            i += 5;

        } else {
            printf("Invalid argument %s.\n", argv[i]);
            exit(1);
        }

    }
    if (table != NULL)
        remove_main_array(table);

#ifdef DLL
    dlclose(handle);
#endif
}

