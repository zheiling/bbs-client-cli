
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
  memset(arr->arr, 0, sizeof(void *) * init_capacity);
}

void u_d_array_append(d_array_ptr_t *arr, void *el_ptr, size_t el_len) {
  u_d_arr_ptr_add(arr, el_ptr, arr->length);
}

void u_d_arr_ptr_add(d_array_ptr_t *arr, void *ptr, int32_t idx) {
  if (idx > arr->capacity - 1) {
    int32_t new_capacity = idx * 2;
    arr->arr = realloc(arr->arr, new_capacity);
    memset(arr->arr + arr->capacity, 0, arr->capacity - new_capacity);
    arr->capacity = new_capacity;
  }
  if (arr->fist_el == NULL)
    arr->fist_el = ptr;

  if (arr->arr[idx] != NULL) {
    memmove(arr->arr + idx + 1, arr->arr + idx, sizeof (void *) * (arr->length - idx));
  }
  arr->last_el = ptr;
  arr->arr[idx] = ptr;
  arr->length = idx + 1;
}

void u_d_arr_ptr_remove_cb(d_array_ptr_t *arr, void *ptr, int32_t idx,
                           u_d_arr_free_callback *callback) {
  void *_ptr = NULL;
  int _idx = idx;

  if (idx > 0) {
    _ptr = arr->arr[idx];
  } else if (ptr != NULL) {
    for (int i = 0; i < arr->length; i++) {
      if (arr->arr[i] == ptr) {
        _ptr = arr->arr[i];
        _idx = i;
      }
    }
  }

  callback(_ptr);
  arr->arr[_idx] = NULL;
  arr->length--;
  memmove(arr->arr + _idx, arr->arr + _idx + 1,
          (arr->length - _idx) * sizeof(void *));
  arr->arr[arr->length] = NULL;
  return;
}

void u_d_arr_ptr_free(d_array_ptr_t *arr) {
  arr->capacity = 0;
  arr->length = 0;
  free(arr->arr);
  arr->arr = NULL;
}

void u_d_arr_free_cb(d_array_ptr_t *d_arr, u_d_arr_free_callback *callback) {
  for (int i = 0; i < d_arr->length; i++) {
    if (d_arr->arr[i] != NULL) {
      callback(d_arr->arr[i]);
    }
  }
  d_arr->capacity = 0;
  d_arr->length = 0;
  free(d_arr->arr);
  d_arr->arr = NULL;
}