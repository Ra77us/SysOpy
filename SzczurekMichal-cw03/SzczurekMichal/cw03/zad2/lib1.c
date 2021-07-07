#define  _GNU_SOURCE
#include "lib1.h"
#include <stdio.h>
#include <ctype.h>


main_array* new_main_array(int size){
    main_array* new_arr = (main_array*)calloc(1, sizeof(main_array));
    new_arr->size = 0;
    new_arr->initial_size = size;
    new_arr->arr = (row_block **)calloc(size, sizeof(row_block*));
    new_arr->curr_id = 0;
    return new_arr;
}

file_pair* new_file_pair(char* first, char* second){
    file_pair* new_pair = (file_pair*)calloc(1,sizeof (file_pair));
    new_pair->first = first;
    new_pair->second = second;
    return new_pair;
}

int line_num(FILE* file){
    if (file == NULL){
        printf("Invalid File\n");
        exit(1);
    }
    int res = 0;
    char *line = NULL;
    size_t len = 0;
    rewind(file);
    while (getline(&line, &len, file) != -1) {
        res += 1;
    }

    free(line);
    rewind(file);
    return res;
}

FILE* merge(file_pair* pair){

    FILE *merged = tmpfile();
    FILE* first = fopen(pair->first, "r");
    FILE* second = fopen(pair->second, "r");
    if (first == NULL || second == NULL)
    {
        printf("File not found!\n");
        exit(1);

    }
    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, first) != -1){
        fprintf(merged, "%s", line);
        if (getline(&line, &len, second) != -1)
            fprintf(merged, "%s", line);
    }
    while (getline(&line, &len, second) != -1){
        fprintf(merged, "%s", line);
    }

    fclose(first);
    fclose(second);
    free(line);

    rewind(merged);
    return merged;
}

int insert_row_block(FILE* tmp, main_array* array){
    if (array->curr_id > array->initial_size - 1){
        printf("Main Array size exceeded.\n");
        exit(1);
    }
    row_block* new_block = (row_block*)calloc(1, sizeof(row_block));
    new_block->initial_size = line_num(tmp);
    new_block->size = new_block->initial_size;
    new_block->text = (char**)calloc(new_block->size, sizeof(char*));

    rewind(tmp);
    size_t len = 0;
    for (int i = 0; i < new_block->initial_size; i++){
        getline(&new_block->text[i], &len, tmp);
    }

    rewind(tmp);

    array->arr[array->curr_id] = new_block;
    array->curr_id += 1;
    array->size += 1;
    return array->curr_id - 1;
}

void print_row_block(row_block* block){
    for (int i=0; i<block->initial_size; i++){
        if (block->text[i] != NULL)
            printf("%s", block->text[i]);
    }
}

void print_main_array(main_array* array){
    for (int i=0; i<array->curr_id; i++){
        if(array->arr[i] != NULL){
            printf("Block %d.:\n",i);
            print_row_block(array->arr[i]);
        }
    }
}

void remove_row(row_block* block, int id){
    if (id<0 || id > block->size){
        printf("row ID out of range.\n");
        exit(1);
    }
    free(block->text[id]);
    block->text[id] = NULL;
    block->size -= 1;
}

void remove_block(main_array* array, int id){
    if (id<0 || id > array->size){
        printf("Block ID out of range.\n");
        exit(1);
    }
    for (int i=0; i<array->arr[id]->initial_size; i++){
        if (array->arr[id]->text[i] != NULL)
            free(array->arr[id]->text[i]);
    }
    free(array->arr[id]->text);
    array->arr[id]->text = NULL;
    free(array->arr[id]);
    array->arr[id] = NULL;
}

int get_size(row_block* block){
    return block->size;
}

void remove_main_array(main_array* array){

    for(int i=0; i<array->initial_size; i++){
        if(array->arr[i] != NULL){
            for (int j=0; j< array->arr[i]->initial_size; j++){
                if (array->arr[i]->text[j] != NULL)
                    free(array->arr[i]->text[j]);
            }
            free(array->arr[i]->text);
            free(array->arr[i]);
        }
    }
    free(array->arr);
    free(array);
}
