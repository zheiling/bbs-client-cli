#include "main.h"
#include "ui/app.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <termios.h>
#include <unistd.h>

void connect_to_server(app_t *app) {
  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = app->params->addr;
  server.sin_port = app->params->port;
  if (-1 ==
      connect(app->params->sd, (struct sockaddr *)&server, sizeof(server))) {
    perror("connect");
    destroy_app(app);
    exit(2);
  }
}

void init_params(params_t *params) {
  params->addr = 0;
  params->pass = NULL;
  params->uname = NULL;
  params->privileges = 0;
  params->port = htons(SERVER_PORT);
}

void clear_params(params_t *params) {
  params->addr = 0;
  params->pass = NULL;
  params->uname = NULL;
  params->privileges = 0;
  params->port = 0;
  free(params->uname);
  free(params->pass);
  params->pass = NULL;
  params->uname = NULL;
}

void get_ip_port(params_t *params, char *ip, char *port) {
  struct sockaddr_in addr;
  inet_aton(ip, &(addr.sin_addr));
  params->addr = addr.sin_addr.s_addr;
  params->port = htons(atoi(port));
}

void close_session(int sd) { close(sd); }

int init_client() {
  int res;
  int cs = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in client;

  client.sin_family = AF_INET;
  client.sin_port = htons(LOCAL_PORT);
  client.sin_addr.s_addr = htonl(INADDR_ANY);

  int opt = 1;
  setsockopt(cs, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  res = bind(cs, (struct sockaddr *)&client, sizeof(client));
  if (res == -1) {
    perror("bind");
    exit(1);
  }
  return cs;
}

void analyze_args(int argc, char *argv[], params_t *params) {
  struct sockaddr_in addr;
  int i;
  for (i = 1; i < argc; i++) {
    if (!strcmp("-p", argv[i])) {
      i++;
      params->port = htons(atoi(argv[i]));
      continue;
    }
    if (!strcmp("-u", argv[i])) {
      i++;
      params->uname = malloc(strlen(argv[i]) + 1);
      strcpy(params->uname, argv[i]);
      continue;
    }
    if (!strcmp("--pass", argv[i])) {
      i++;
      params->pass = malloc(strlen(argv[i]) + 1);
      strcpy(params->pass, argv[i]);
      continue;
    }
    if (!strcmp("-h", argv[i])) {
      i++;
      inet_aton(argv[i], &(addr.sin_addr));
      params->addr = addr.sin_addr.s_addr;
      continue;
    }
  }
}