#ifndef D_UTILS_H
#define D_UTILS_H
#include "d_array.h"
#include "utf8.h"

/* Free if not null and assign NULL */
#define FREE_MLC(arg)                                                          \
  if (arg != NULL) {                                                           \
    free(arg);                                                                 \
    arg = NULL;                                                                \
  }
#endif