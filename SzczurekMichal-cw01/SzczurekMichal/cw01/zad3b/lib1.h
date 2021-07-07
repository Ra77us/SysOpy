#ifndef LIB1_H
#define LIB1_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct row_block{
    int initial_size; //capacity
    int size; // number of actual rows
    char** text;
}row_block;

typedef struct main_array{
    int initial_size; //capacity
    int size; //number of actual blocks
    int curr_id; // id for next block
    row_block **arr;
}main_array;

typedef struct file_pair{
    char* first;
    char* second;
}file_pair;

main_array* new_main_array(int size);
file_pair* new_file_pair(char* first, char* second);

int insert_row_block(FILE* tmp, main_array* arr);
int line_num(FILE* file);
FILE* merge(file_pair* pair);
void print_row_block(row_block* block);
void print_main_array(main_array* array);
int get_size(row_block* block);
void remove_row(row_block* block, int id);
void remove_block(main_array* array, int id);
void remove_main_array(main_array* array);
#endif //LIB1_H
