/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#ifndef QUERY_H
#define QUERY_H

#include "../app.h"
#include "main.h"
#include <widget.h>

void query_loop(app_t *);
void wait_register(query_args_t *q_args);
int process_query(app_t *app);
int query_extract_from_buf(char *buf, int *buf_used, char **output_line);
void init_query_args(query_args_t *q_args, params_t *params);
typedef int wait_server_cb(app_t *app, char *query, int q_len);
void query_return_to_buf(char *buf, int *buf_used, char *input_line);
#endif