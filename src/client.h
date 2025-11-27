#ifndef CLIENT_H
#define CLIENT_H
#include "main.h"

int process_client_command(char *line, int l_len, query_args_t *q_args);
void print_prompt(params_t *params);

#endif