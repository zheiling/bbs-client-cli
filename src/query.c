#include "main.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

static void init_query_args(query_args_t *q_args, int sd, char *buf,
                            enum state state, int buf_used, params_t *params);
static void wait_side(query_args_t *q_args);
void user_request_description(query_args_t *q_args);

void query_loop(int sd, params_t *params) {
  fd_set readfds;

  size_t qlen;
  int sr;
  char buf[INBUFSIZE];
  char *bufs = buf;
  static file_args_t file_args;
  static query_args_t query_args;

  init_query_args(&query_args, sd, buf, WAIT_SERVER_INIT, 0, params);
  init_file_args(&file_args);

  for (;;) {
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    FD_SET(sd, &readfds);
    int maxfd = sd;
    if (query_args.state == STATE_UPLOAD_FILE) {
      if (query_args.file != NULL) {
        FD_SET(query_args.file->fd, &readfds);
        if (query_args.file->fd > maxfd)
          maxfd = query_args.file->fd;
      }
    }
    sr = select(maxfd + 1, &readfds, NULL, NULL, NULL);
    if (sr == -1) {
      perror("select");
      exit(3);
    }

    if (FD_ISSET(sd, &readfds)) {
      /* process request from the server */
      qlen = read(sd, buf, INBUFSIZE);
      if (qlen == 0) {
        close_session(sd);
        printf("\n*** server closed the connection. ***\n");
        exit(1);
      } else {
        query_args.buf_used = qlen;
        query_args.from_server = 1;
        if (-1 == process_query(&query_args, &file_args)) {
          close_session(sd);
          exit(1);
        }
      }
    }

    if (query_args.file && query_args.file->fd > -1 &&
        FD_ISSET(query_args.file->fd, &readfds)) {
      /* process upload/download */
      query_args.buf_used = read(query_args.file->fd, buf, INBUFSIZE);
      process_query(&query_args, &file_args);
    }

    if (FD_ISSET(STDIN_FILENO, &readfds)) {
      /* process request from the client */
      qlen = read(STDIN_FILENO, buf, INBUFSIZE);
      if (qlen == 0) {
        close_session(sd);
        exit(0);
      } else {
        query_args.buf_used = qlen;
        query_args.from_server = 0;
        if (-1 == process_query(&query_args, &file_args)) {
          close_session(sd);
          exit(1);
        }
      }
    }
  }
}

int process_query(query_args_t *query_args, file_args_t *file_args) {
  query_args->buf[query_args->buf_used] = 0;

  switch (query_args->state) {
  case WAIT_SERVER:
  case WAIT_SERVER_INIT:
  case WAIT_CLIENT:
    wait_side(query_args);
    break;
  case UPLOAD_FILE:
  case ERR:
    break;
  case STATE_FILE_LIST:
    file_list(file_args, query_args);
    break;
  case STATE_FILE_SELECT:
    file_select(file_args, query_args);
    break;
  case STATE_FILE_DOWNLOAD:
    file_download(file_args, query_args);
    break;
  case STATE_UPLOAD_REQUESTED:
    if (!file_upload_start(query_args)) {
      query_args->state = STATE_UPLOAD_FILE;
    } else {
      print_prompt(query_args->params);
      query_args->state = WAIT_CLIENT;
    }
    break;
  case STATE_UPLOAD_PARAMS:
  case STATE_UPLOAD_FILE:
    if (file_upload(query_args)) {
      /* upload finishes */
      user_request_description(query_args);
      print_prompt(query_args->params);
      query_args->state = WAIT_CLIENT;
    }
    break;
  }
  return 0;
}

int query_extract_from_buf(char *buf, int *buf_used, char **output_line) {
  char *line;
  int pos = -1;

  if (*buf_used > 0) {
    char *cptr = strchr(buf, '\n');
    if (cptr != NULL)
      pos = cptr - buf;
  } else {
    return 0;
  }

  if (pos == -1) {
    int b_used = *buf_used;
    *buf_used = 0;
    line = malloc(b_used + 1);
    strncpy(line, buf, b_used);

    // line[b_used] = 0;

    *output_line = line;
    return b_used;
  } else {
    line = malloc(pos + 2);
    strncpy(line, buf, pos + 1);
    line[++pos] = 0;
    *buf_used -= (pos);
    if (!pos)
      pos++;
    memmove(buf, buf + pos, *buf_used);
    buf[*buf_used] = 0;
    if (line[pos-1] == '\r')
      line[--pos] = 0;
    *output_line = line;
  }
  return pos + 1;
}

static void wait_side(query_args_t *q_args) {
  int qlen;
  int buf_used = q_args->buf_used;
  char *query = NULL;

  if (q_args->state == WAIT_SERVER || q_args->state == WAIT_SERVER_INIT || q_args->from_server) {
    while ((qlen = query_extract_from_buf(q_args->buf, &buf_used, &query))) {
      process_server_command(query, qlen, q_args);
      free(query);
      query = NULL;
    }
  } else if (q_args->state == WAIT_CLIENT) {
    while ((qlen = query_extract_from_buf(q_args->buf, &buf_used, &query))) {
      process_client_command(query, qlen, q_args);
      free(query);
      query = NULL;
    }
  }
}

static void init_query_args(query_args_t *q_args, int sd, char *buf,
                            enum state state, int buf_used, params_t *params) {
  q_args->buf = buf;
  q_args->sd = sd;
  q_args->state = state;
  q_args->buf_used = buf_used;
  q_args->params = params;
  q_args->file = NULL;
  q_args->from_server = 0;
}

void user_request_description(query_args_t *q_args) {
  printf("File description > ");
  fflush(stdout);
  char buf[INBUFSIZE]; /* пока ограничение на буфер */
  char *line_buf = NULL;
  int blen = 0;
  int line_len = 0;
  size_t ll;
  while (blen < INBUFSIZE - 6) { /* reserve space for ":END:\n" */
    line_len = getline(&line_buf, &ll, stdin);
    if (line_len == 1) {
      /* free(line); -> не работает */
      break;
    }
    if (blen == 0) {
      strcpy(buf, line_buf);
    } else {
      strcat(buf, line_buf);
    }
    free(line_buf);
    line_buf = NULL;
    blen += line_len;
  }

  strcat(buf, ":END:\n");
  blen += 6;
  write(q_args->sd, buf, blen);
}