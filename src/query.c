/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#include "query.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <ncursesw/ncurses.h>
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

#include <modals.h>
#include <widget.h>
#include <widget_core.h>

#include "alert.h"
#include "connection.h"
#include "dialogue.h"
#include "file_processor.h"
#include "main.h"
#include "server.h"
#include "types.h"

static int process_command(app_t *app, wait_server_cb *cb);
void user_request_description(query_args_t *q_args);
int process_query(app_t *app);
int32_t process_user_input(app_t *app, callback_args_t *d_args);

#define EXIT_APP(message, app, exit_code)                                      \
  {                                                                            \
    alert(message);                                                            \
    close_session(app->params->sd);                                            \
    destroy_app(app, exit_code);                                               \
  }

void query_loop(app_t *app) {
  query_args_t *query_args = app->query_args;
  fd_set readfds;
  int32_t sd = app->params->sd;
  size_t qlen;
  int sr;
  static file_args_t file_args;
  callback_args_t d_args = {
      .app = app, .element = NULL, .data = NULL, .resp_data.code = cbrc_none};

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
    if (query_args->state == S_NEXT_ACTION) {
      query_args->from_server = TRUE;
      if (ERR == process_query(app)) {
        EXIT_APP("*** error while processing query ***", app, 1)
      }
      query_args->state = S_WAIT_SERVER;
      continue;
    } else {
      sr = select(maxfd + 1, &readfds, NULL, NULL, NULL);
      if (sr == -1) {
        /* perror("select"); */
        EXIT_APP("*** error while processing 'select' call ***", app, 3)
      }
    }

    if (FD_ISSET(sd, &readfds)) {
      /* process request from the server */
      qlen = read(sd, query_args->buf, INBUFSIZE);
      if (qlen == 0) {
        EXIT_APP("*** server closed the connection ***", app, 1)
      } else {
        query_args->buf_used = qlen;
        query_args->from_server = TRUE;
        if (ERR == process_query(app)) {
          EXIT_APP("*** error while processing query ***", app, 1)
        }
      }
    }

    if (query_args->file && query_args->file->fd > -1 &&
        FD_ISSET(query_args->file->fd, &readfds)) {
      /* process upload/download */
      query_args->buf_used =
          read(query_args->file->fd, query_args->buf, INBUFSIZE);
      process_query(app);
    }

    if (FD_ISSET(STDIN_FILENO, &readfds)) {
      if (ERR == process_user_input(app, &d_args)) {
        EXIT_APP("*** error while processing user input ***", app, 4)
      }
    }
  }
}

void wait_register(query_args_t *q_args) {
  params_t *params = q_args->params;
  q_args->state = WAIT_REGISTER_CONFIRMATION;
  char email[EMAIL_LEN];
  ask_register(q_args->params, email);
  server_send_string(q_args, "register %s %s %s\n", params->uname, params->pass,
                     email);
}

int upload_confirm_cb(app_t *app, char *query, int q_len) {
  if (!strncmp("finished\n", query, sizeof("finished\n") - 1)) {
    ui_file_list_t *fui = (ui_file_list_t *)app->query_args->main_ui->ui;
    notification("File upload", dc_normal, "File %s is uploaded to the server!",
                 app->query_args->file->name);
    clear_file_in_query(app->query_args);
    app->query_args->state = S_FILE_LIST;
    app->modal.needs_destroy = true;
    reset_file_list(fui);
    server_send_string(app->query_args, "file list %u %u\n", fui->max_lines, 1);
    return 0;
  }
  return 1; /* TODO: Error case */
}

int process_query(app_t *app) {
  query_args_t *query_args = app->query_args;
  file_args_t *file_args = app->file_args;
  query_args->buf[query_args->buf_used] = 0;
  int32_t res = 0;
  switch (query_args->state) {
  case WAIT_SERVER:
  case WAIT_SERVER_INIT:
  case WAIT_CLIENT:
  case WAIT_REGISTER_CONFIRMATION:
  case S_WAIT_PASS:
  case S_WAIT_SERVER:
    process_command(app, NULL);
    break;
  case S_NEXT_ACTION:
    process_command(app, NULL);
    break;
  case S_WAIT_REGISTER_CONFIRMATION:
    process_command(app, server_wait_reg_confirm_cb);
    break;
  case S_ERR:
    break;
  case S_FILE_LIST:
    file_list(file_args, query_args);
    break;
  case S_FILE_DOWNLOAD:
    file_download(file_args, query_args);
    break;
  case S_UPLOAD_REQUESTED:
    if (!file_upload_start(query_args)) {
      query_args->state = S_UPLOAD_FILE;
    } else {
      query_args->state = WAIT_CLIENT;
    }
    break;
  case S_UPLOAD_PARAMS:
  case S_UPLOAD_FILE:
    if ((res = file_upload(query_args)) == 1) {
      /* wait server for finish upload */
      query_args->state = S_UPLOAD_SERVER_FINISHES;
    } else if (res == -1) {
      /* TODO: Error */
    }
    break;
  case S_UPLOAD_SERVER_FINISHES:
    process_command(app, upload_confirm_cb);
    /* user_request_description(query_args); */
    break;
  case S_ASK_USER_BEFORE_LOGIN:
    break;
  case WAIT_REGISTER:
    wait_register(query_args);
    break;
  case S_PREP_SERVER_MESSAGE:
    process_command(app, server_print_message_cb);
    break;
  default:
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

    line[b_used] = 0;

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

void query_return_to_buf(char *buf, int *buf_used, char *input_line) {
  ssize_t l_len = strlen(input_line);
  memmove(buf + l_len, buf, l_len);
  memcpy(buf, input_line, l_len);
  *buf_used = l_len;
  buf[*buf_used] = 0;
}

static int process_command(app_t *app, wait_server_cb *callback) {
  query_args_t *q_args = app->query_args;
  int qlen;
  char *query = NULL;
  int res = 0;

  while ((qlen = query_extract_from_buf(q_args->buf, &(q_args->buf_used),
                                        &query))) {
    if (callback == NULL) {
      process_server_command(query, qlen, app);
    } else {
      res = callback(app, query, qlen);
    }
    free(query);
    query = NULL;
    if (res == 1) {
      break;
    }
  }
  return res;
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
  q_args->progress_bar = NULL;
  q_args->active_dialogue = NULL;
  q_args->notification = NULL;
}

void user_request_description(query_args_t *q_args) {
  server_send_string(q_args, "%s\n\n:END:\n", q_args->file->description);
}