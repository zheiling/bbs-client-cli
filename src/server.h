/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#ifndef SERVER_H
#define SERVER_H

#include "app.h"
#include "main.h"
int  process_server_command(char *line, int l_len, app_t *app);
void ask_register(params_t *params, char *email);
void server_send_string(query_args_t *q, const char *fmt, ...);
int  server_print_message_cb(app_t *app, char *line, int l_len);
int  server_wait_reg_confirm_cb(app_t *app, char *line, int l_len);
#endif