#ifndef SERVER_H
#define SERVER_H

#include "app.h"
#include "main.h"
int process_server_command(char *line, int l_len, app_t *app);
void ask_register(params_t *params, char *email);

#endif