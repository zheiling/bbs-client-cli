#include "main.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>

void connect_to_server(int sd, params_t *params) {
  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = params->addr;
  server.sin_port = params->port;
  if (-1 == connect(sd, (struct sockaddr *)&server, sizeof(server))) {
    perror("connect");
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

void get_missing_params(params_t *params) {
  size_t lsize;
  char *buf = NULL;
  struct termios ts_hide, ts_show;
  struct sockaddr_in addr;

  if (isatty(0)) {
    tcgetattr(0, &ts_show);
    memcpy(&ts_hide, &ts_show, sizeof(ts_show));
    ts_hide.c_lflag &= ~ECHO;
  }

  if (!params->addr) {
    printf("host> ");
    getline(&buf, &lsize, stdin);
    inet_aton(buf, &(addr.sin_addr));
    params->addr = addr.sin_addr.s_addr;
    free(buf);
    buf = NULL;
  }
  if (params->uname == NULL) {
    printf("login> ");
    getline(&(params->uname), &lsize, stdin);
    lsize = strlen(params->uname);
    params->uname[lsize - 1] = '\0'; /* get rid of '\n' */
    free(buf);
    buf = NULL;
  }
  if (params->pass == NULL) {
    if (isatty(0)) {
      tcsetattr(0, TCSANOW, &ts_hide);
    }
    printf("password> ");
    getline(&(params->pass), &lsize, stdin);
    if (isatty(0)) {
      tcsetattr(0, TCSANOW, &ts_show);
      putchar('\n');
    }
    lsize = strlen(params->pass);
    params->pass[lsize - 1] = '\0'; /* get rid of '\n' */
    free(buf);
    buf = NULL;
  }
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