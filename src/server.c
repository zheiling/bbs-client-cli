#include "main.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int process_server_command(char *line, int l_len, query_args_t *q_args) {
  int ws_pos = l_len;
  params_t *params = q_args->params;

  char *cptr = strchr(line, ' ');
  if (cptr != NULL && cptr > line)
    ws_pos = cptr - line;

  /* LOGIN */
  if (!strncmp(line, "login>", ws_pos)) {
    write(q_args->sd, params->uname, strlen(params->uname));
    return 0;
  }

  /* PASSWORD */
  if (!strncmp(line, "password>", ws_pos)) {
    write(q_args->sd, params->pass, strlen(params->pass));
    return 0;
  }

  /* LOGIN AGAIN */
  if (!strncmp(line, "login_again>", ws_pos)) {
    free(params->uname);
    free(params->pass);
    params->uname = NULL;
    params->pass = NULL;
    get_missing_params(params);
    write(q_args->sd, params->uname, strlen(params->uname));
    return 0;
  }

  /* DOWNLOAD START */
  if (!strncmp(line, "download_start", ws_pos)) {
    return 0;
  }

  /* WELCOME MES */
  if (!strncmp(line, "Welcome, ", ws_pos)) {
    print_prompt(q_args->params);
    q_args->state = WAIT_CLIENT;
    return 0;
  }

  if (q_args->state == WAIT_SERVER_INIT) {
    write(STDOUT_FILENO, line, l_len);
  } else {
    char buf[INBUFSIZE + sizeof("server: ")] = "server: ";
    strcat(buf, line);
    write(STDOUT_FILENO, buf, l_len + sizeof("server: "));
    if (q_args->state == WAIT_CLIENT) {
      print_prompt(q_args->params);
    }
  }

  return 0;
}

static void file_receive(int sd, char *line) {
  char fname[128]; char command[32]; size_t fsize;
  sscanf(line, "%s %s %zu", command, fname, &fsize);
  char buf[INBUFSIZE];
  int rlen;

  int file_d = open(fname, O_WRONLY);

  if (file_d == -1) {
    char err_mes[256];
    int mlen;
    mlen = sprintf(err_mes, "Can't write file named \"%s\"\n", fname);
    write(sd, err_mes, mlen);
    close(file_d);
    return;
  }

  while (fsize && (rlen = read(sd, buf, INBUFSIZE))) {
    write(file_d, buf, rlen);
    fsize -= rlen;
  }

  close(file_d);
}