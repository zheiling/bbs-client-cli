#ifndef D_ARRAY_H
#define D_ARRAY_H
/* Dynamic array of numbers */
#include <stdint.h>
typedef struct d_array_ptr_t {
  int64_t capacity;
  int64_t length;
  void **arr;
  void *fist_el;
  void *last_el;
} d_array_ptr_t;

void init_d_arr_ptr(d_array_ptr_t *arr, int64_t init_capacity);
void add_d_arr_ptr(d_array_ptr_t *arr, void *ptr, int64_t idx);
void free_d_arr(d_array_ptr_t *arr);

#endif