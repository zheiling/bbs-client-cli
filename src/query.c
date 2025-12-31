#include <arpa/inet.h>
#include <fcntl.h>
#include <ncurses.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
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

#include "client.h"
#include "connection.h"
#include "file_processor.h"
#include "main.h"
#include "server.h"
#include "ui/app.h"

static void wait_side(query_args_t *q_args);
void user_request_description(query_args_t *q_args);
int process_query(query_args_t *query_args, file_args_t *file_args);
int32_t process_user_input(app_t *app, callback_args_t *d_args);

void query_loop(app_t *app) {
  query_args_t *query_args = app->query_args;
  fd_set readfds;
  int32_t sd = app->params->sd;
  size_t qlen;
  int sr;
  static file_args_t file_args;
  callback_args_t d_args = {
      .app = app, .widget = NULL, .data = NULL, .resp_data = NULL};

  init_file_args(&file_args);
  query_args->sd = sd;
  app->file_args = &file_args;

  for (;;) {
    /* update screen */
    app_draw_modal(app);
    app_refresh(app);

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    FD_SET(sd, &readfds);
    int maxfd = app->params->is_connected ? sd : STDIN_FILENO;
    if (query_args->state == S_UPLOAD_FILE) {
      if (query_args->file != NULL) {
        FD_SET(query_args->file->fd, &readfds);
        if (query_args->file->fd > maxfd)
          maxfd = query_args->file->fd;
      }
    }
    if (query_args->state == S_NEXT_ACTION &&
        query_args->next_server_command != NULL) {
      query_args->buf_used = strlen(query_args->next_server_command);
      memcpy(query_args->buf, query_args->next_server_command,
             query_args->buf_used);
      free(query_args->next_server_command);
      query_args->next_server_command = NULL;
      query_args->from_server = TRUE;
      if (ERR == process_query(query_args, &file_args)) {
        close_session(sd);
        exit(1);
      }
      continue;
    } else {
      sr = select(maxfd + 1, &readfds, NULL, NULL, NULL);
      if (sr == -1) {
        perror("select");
        exit(3);
      }
    }

    if (FD_ISSET(sd, &readfds)) {
      /* process request from the server */
      qlen = read(sd, query_args->buf, INBUFSIZE);
      if (qlen == 0) {
        close_session(sd);
        printf("\n*** server closed the connection. ***\n");
        exit(1);
      } else {
        query_args->buf_used = qlen;
        query_args->from_server = TRUE;
        if (ERR == process_query(query_args, &file_args)) {
          close_session(sd);
          exit(1);
        }
      }
    }

    if (query_args->file && query_args->file->fd > -1 &&
        FD_ISSET(query_args->file->fd, &readfds)) {
      /* process upload/download */
      query_args->buf_used =
          read(query_args->file->fd, query_args->buf, INBUFSIZE);
      process_query(query_args, &file_args);
    }

    if (FD_ISSET(STDIN_FILENO, &readfds)) {
      /* process request from the client */
      /* qlen = read(STDIN_FILENO, buf, INBUFSIZE); */
      /* if (qlen == 0) {
        close_session(sd);
        exit(0);
      } else { */
      /*       query_args->buf_used = qlen;
            query_args->from_server = 0; */
      // if (-1 == process_query(&query_args, &file_args)) {
      if (ERR == process_user_input(app, &d_args)) {
        close_session(sd);
        exit(1);
      }
    }
  }
}

void wait_register(query_args_t *q_args) {
  params_t *params = q_args->params;
  char r_buf[INBUFSIZE];
  q_args->state = WAIT_REGISTER_CONFIRMATION;
  char email[EMAIL_LEN];
  ask_register(q_args->params, email);
  sprintf(r_buf, "register %s %s %s\n", params->uname, params->pass, email);
  write(q_args->sd, r_buf, strlen(r_buf) - 1);
}

int process_query(query_args_t *query_args, file_args_t *file_args) {
  query_args->buf[query_args->buf_used] = 0;

  switch (query_args->state) {
  case WAIT_SERVER:
  case WAIT_SERVER_INIT:
  case WAIT_CLIENT:
  case S_NEXT_ACTION:
  case S_WAIT_SERVER:
  case WAIT_REGISTER_CONFIRMATION:
    wait_side(query_args);
    break;
  case UPLOAD_FILE:
  case S_ERR:
    break;
  case S_FILE_LIST:
    file_list(file_args, query_args);
    break;
  case S_FILE_SELECT:
    if (query_args->from_server) {
      wait_side(query_args);
    } else {
      file_select(file_args, query_args);
    }
    break;
  case S_FILE_DOWNLOAD:
    file_download(file_args, query_args);
    break;
  case S_UPLOAD_REQUESTED:
    if (!file_upload_start(query_args)) {
      query_args->state = S_UPLOAD_FILE;
    } else {
      print_prompt(query_args->params);
      query_args->state = WAIT_CLIENT;
    }
    break;
  case S_UPLOAD_PARAMS:
  case S_UPLOAD_FILE:
    if (file_upload(query_args)) {
      /* upload finishes */
      user_request_description(query_args);
      print_prompt(query_args->params);
      query_args->state = WAIT_CLIENT;
    }
    break;
  case S_ASK_USER_BEFORE_LOGIN:
    break;
  case WAIT_REGISTER:
    wait_register(query_args);
    break;
  case S_ASK_SEVER_IP:
  case S_ASK_LOGIN_TYPE:
  case S_N_D:
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
    if (line[pos - 1] == '\r')
      line[--pos] = 0;
    *output_line = line;
  }
  return pos + 1;
}

static void wait_side(query_args_t *q_args) {
  int qlen;
  int buf_used = q_args->buf_used;
  char *query = NULL;

  if (q_args->state == WAIT_SERVER || q_args->state == WAIT_SERVER_INIT ||
      q_args->from_server || q_args->state == S_WAIT_SERVER) {
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

void init_query_args(query_args_t *q_args, params_t *params) {
  q_args->buf = NULL;
  q_args->sd = -1;
  q_args->state = S_N_D;
  q_args->buf_used = 0;
  q_args->params = params;
  q_args->file = NULL;
  q_args->from_server = 0;
  q_args->server_message.text = NULL;
  q_args->server_message.capacity = 0;
  q_args->server_message.size = 0;
  q_args->next_server_command = NULL;
  q_args->progress_bar = NULL;
  q_args->active_dialogue = NULL;
  q_args->notification = NULL;
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