/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#ifndef FILE_PROCESSOR_H
#define FILE_PROCESSOR_H

#include "common.h"

void file_list(file_args_t *f_args, query_args_t *q_args);
void file_download(file_args_t *f_args, query_args_t *q_args);
void init_file_args(file_args_t *fargs, app_t *app);
int file_upload_request(char *, query_args_t *q_args);
int file_upload_start(query_args_t *q_args);
int file_upload(query_args_t *q_args);
int32_t file_upload_open(char *dpath, char *fname, query_args_t *q_args);
void fl_clear(fl_item_t **start, fl_item_t **arg_current);
int32_t ui_file_select(file_args_t *f_args, query_args_t *q_args, fl_item_t *ptr);
void clear_file_in_query(query_args_t *q_args);

#endif