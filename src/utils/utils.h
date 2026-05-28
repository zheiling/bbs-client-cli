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

#define FILE_CLEAN(app)                                                        \
  if (app->query_args->file != NULL) {                                         \
    FREE_MLC(app->query_args->file->name);                                     \
    FREE_MLC(app->query_args->file->path);                                     \
    FREE_MLC(app->query_args->file);                                           \
  }
#endif