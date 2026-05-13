#ifndef D_ARRAY_H
#define D_ARRAY_H
/* Dynamic array of numbers */
#include <stdint.h>
typedef struct d_array_ptr_t {
  int32_t capacity;
  int32_t length;
  void **arr;
  void *fist_el;
  void *last_el;
} d_array_ptr_t;

void u_d_arr_ptr_init(d_array_ptr_t *arr, int32_t init_capacity);
void u_d_arr_ptr_add(d_array_ptr_t *arr, void *ptr, int32_t idx);
void u_d_arr_ptr_free(d_array_ptr_t *arr);

#endif