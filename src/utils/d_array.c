
#include "d_array.h"
#include <stdint.h>
#include <stdlib.h>

void init_d_arr_ptr(d_array_ptr_t *arr, int64_t init_capacity) {
  arr->capacity = init_capacity;
  arr->length = 0;
  arr->arr = malloc(sizeof(void *) * init_capacity);
}

void add_d_arr_ptr(d_array_ptr_t *arr, void *ptr, int64_t idx) {
  if (idx > arr->capacity - 1) {
    arr->capacity = idx * 2;
    arr->arr = realloc(arr->arr, arr->capacity);
  }
  arr->arr[idx] = ptr;
  arr->length = idx+1;
}

void free_d_arr(d_array_ptr_t *arr) {
  arr->capacity = 0;
  arr->length = 0;
  free(arr->arr);
  arr->arr = NULL;
}