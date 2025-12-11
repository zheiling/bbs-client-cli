#ifndef CONNECTION_H
#define CONNECTION_H
#include "main.h"

void connect_to_server(int sd, params_t *params);
void init_params(params_t *restrict);
void clear_params(params_t *params);
void get_ip_port(params_t *params, char *ip, char *port);
void close_session(int sd);
int init_client();
void analyze_args(int argc, char *argv[], params_t *restrict);

#endif