#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct DynamicArray {
    char* data;
}DynamicArray;

void init_array(DynamicArray* dynamicArray);
void push_back_item(DynamicArray* dynamicArray, const char* newChar);
void delete_item(DynamicArray* dynamicArray, int index);
void free_array(DynamicArray* dynamicArray);


#endif // DYNAMIC_ARRAY_H
