#ifndef QUERY_H
#define QUERY_H

#include "main.h"
#include "ui/app.h"

void query_loop(app_t *);
void wait_register(query_args_t *q_args);
int process_query(query_args_t *query_args, file_args_t *file_args);
int query_extract_from_buf(char *buf, int *buf_used, char **output_line);
void init_query_args(query_args_t *q_args, params_t *params);

#endif