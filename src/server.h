#ifndef SERVER_H
#define SERVER_H

#include "main.h"
int process_server_command(char *line, int l_len, query_args_t *q_args);
void ask_register(params_t *params, char *email);

#endif