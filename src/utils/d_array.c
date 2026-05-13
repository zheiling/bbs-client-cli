
#include "d_array.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void u_d_arr_ptr_init(d_array_ptr_t *arr, int32_t init_capacity) {
  arr->capacity = init_capacity;
  arr->length = 0;
  arr->arr = malloc(sizeof(void *) * init_capacity);
  arr->fist_el = NULL;
  arr->last_el = NULL;
  memset(arr->arr, 0, init_capacity);
}

void u_d_arr_ptr_add(d_array_ptr_t *arr, void *ptr, int32_t idx) {
  int32_t new_capacity = idx * 2;
  if (idx > arr->capacity - 1) {
    arr->arr = realloc(arr->arr, new_capacity);
    memset(arr->arr+arr->capacity, 0, arr->capacity - new_capacity);
    arr->capacity = new_capacity;
  }
  if (arr->fist_el == NULL) arr->fist_el = ptr;
  arr->last_el = ptr;
  arr->arr[idx] = ptr;
  arr->length = idx+1;
}

void u_d_arr_ptr_free(d_array_ptr_t *arr) {
  arr->capacity = 0;
  arr->length = 0;
  free(arr->arr);
  arr->arr = NULL;
}