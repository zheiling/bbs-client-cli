#ifndef COMMON_H
#define COMMON_H
#include <stdint.h>
typedef struct {
  uint32_t cur_y, cur_x;
  uint32_t max_y, max_x;
} coordinates_t;
#endif