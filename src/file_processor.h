#ifndef FILE_PROCESSOR_H
#define FILE_PROCESSOR_H
#include "main.h"

void file_list(file_args_t *f_args, query_args_t *q_args);
void file_download(file_args_t *f_args, query_args_t *q_args);
void file_select(file_args_t *fargs, query_args_t *q_args);
void init_file_args(file_args_t *fargs);
int file_upload_request(char *, query_args_t *q_args);
int file_upload_start(query_args_t *q_args);
int file_upload(query_args_t *q_args);

#endif