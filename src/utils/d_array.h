#ifndef D_ARRAY_H
#define D_ARRAY_H
/* Dynamic array of numbers */

#include <stddef.h>
#include <stdint.h>

typedef struct d_array_ptr_t {
  int32_t capacity;
  int32_t length;
  void **arr;
  void *fist_el;
  void *last_el;
} d_array_ptr_t;

typedef void *u_d_arr_free_callback(void *arg);

void u_d_arr_ptr_init(d_array_ptr_t *arr, int32_t init_capacity);
void u_d_arr_ptr_add(d_array_ptr_t *arr, void *ptr, int32_t idx);
void u_d_arr_ptr_free(d_array_ptr_t *arr);
void u_d_array_append(d_array_ptr_t *arr, void *el_ptr, size_t el_len);
void u_d_arr_free_cb(d_array_ptr_t *arr, u_d_arr_free_callback *callback);

#endif