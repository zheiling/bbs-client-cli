/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#include "app.h"
#include "dialogue.h"
#include "main.h"
#include "query.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <modals.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <termios.h>
#include <unistd.h>
#include <widget.h>

void ask_uname_and_password(params_t *params);
void ask_register(params_t *params, char *email);

void server_send_string(query_args_t *q, const char *fmt, ...) {
  char buf[INBUFSIZE];
  size_t b_len = 0;
  va_list args;
  va_start(args, fmt);
  b_len = vsprintf(buf, fmt, args);
  write(q->sd, buf, b_len);
  va_end(args);
}

int server_wait_reg_confirm_cb(app_t *app, char *line, int l_len) {
  query_args_t *q_args = app->query_args;
  char query[INBUFSIZE];
  ui_file_list_t *fui = (ui_file_list_t *)q_args->file_list_ui;

  if (!strcmp(line, "ok\n")) {
    server_send_string(q_args, "file list %u %u\n", fui->max_lines,
                       fui->current_page);
    q_args->state = S_FILE_LIST;
    destroy_dialogue(&(app->modal), app);
    sprintf(query,
            "You've been successfully registered.\n"
            "Welcome, %s!",
            app->params->uname);
    notification("Registration", dc_normal, query);
    print_bars(app);
    return 0;
  } else {
    q_args->state = S_ASK_REGISTER;
    alert(line);
    return 1;
  }
}

int server_print_message_cb(app_t *app, char *line, int l_len) {
  int64_t new_capacity;
  query_args_t *q_args = app->query_args;
  if (q_args->server_message.text == NULL) {
    q_args->server_message.text = malloc(l_len + 1);
    memcpy(q_args->server_message.text, line, l_len);
    q_args->server_message.text[l_len] = 0;
    q_args->server_message.size = l_len + 1;
    q_args->server_message.capacity = l_len + 1;
  } else {
    if (q_args->server_message.size + l_len > q_args->server_message.capacity) {
      new_capacity = (l_len + q_args->server_message.capacity) * 2;
      q_args->server_message.text =
          realloc(q_args->server_message.text, new_capacity);
      q_args->server_message.capacity = new_capacity;
    }
    q_args->server_message.size += l_len;
    strncat(q_args->server_message.text, line, l_len);
    q_args->server_message.text[q_args->server_message.size + l_len] = 0;
  }
  char *end_char = strchr(q_args->server_message.text,
                          '\04'); /* Search for the end of the notification */
  if (end_char != NULL) {
    query_return_to_buf(app->query_args->buf, &(app->query_args->buf_used),
                        end_char + 1);
    *end_char = '\0'; /* Don't show this symbol */
    notification("Server response", dc_normal, q_args->server_message.text);
    /* q_args->state = S_PRINT_SERVER_MESSAGE; */
    q_args->server_message.size = 0;
    q_args->server_message.text[0] = 0;
    q_args->state = S_NEXT_ACTION;
    return 1;
  }
  return 0;
}

/* TODO: refactor to process callbacks */
int process_server_command(char *line, int l_len, app_t *app) {
  int ws_pos = l_len;
  query_args_t *q_args = app->query_args;
  params_t *params = q_args->params;
  ui_file_list_t *fui = (ui_file_list_t *)q_args->file_list_ui;

  char *cptr = strchr(line, ' ');
  if (cptr != NULL && cptr > line)
    ws_pos = cptr - line;

  /* LOGIN */
  if (!strncmp(line, "login>", ws_pos)) {
    if (q_args->params->uname != NULL && q_args->params->pass != NULL) {
      server_send_string(q_args, q_args->params->uname);
    } else {
      q_args->state = S_ASK_LOGIN_TYPE;
    }
    return 0;
  }

  /* PASSWORD */
  if (!strncmp(line, "password>", ws_pos)) {
    if (q_args->params->pass == NULL) {
      return 1;
    }
    server_send_string(q_args, params->pass);
    return 0;
  }

  /* LOGIN AGAIN */
  if (!strncmp(line, "login_again>\n", ws_pos)) {
    alert("Incorrect credentials! Try again");
    free(params->uname);
    free(params->pass);
    params->uname = NULL;
    params->pass = NULL;
    q_args->state = S_ASK_LOGIN_USER;
    return 0;
  }

  /* DOWNLOAD START */
  if (!strncmp(line, "download_start", ws_pos)) {
    return 0;
  }

  /* WELCOME MES */
  if (!strncmp(line, "Welcome, ", ws_pos)) {
    server_send_string(q_args, "file list %u %u\n", fui->max_lines,
                       fui->current_page);
    q_args->state = S_FILE_LIST;
    return 0;
  }

  return -1;
}

void ask_uname_and_password(params_t *params) {
  size_t lsize;
  char *bufptr = NULL;

  struct termios ts_hide, ts_show;

  if (isatty(0)) {
    tcgetattr(0, &ts_show);
    memcpy(&ts_hide, &ts_show, sizeof(ts_show));
    ts_hide.c_lflag &= ~ECHO;
  }

  if (params->uname == NULL) {
    printf("login> ");
    fflush(stdout);
    getline(&(params->uname), &lsize, stdin);
    lsize = strlen(params->uname);
    params->uname[lsize - 1] = '\0'; /* get rid of '\n' */
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
    free(bufptr);
    bufptr = NULL;
  }
}

void ask_register(params_t *params, char *email) {
  size_t lsize;
  char *bufptr = NULL;

  struct termios ts_hide, ts_show;

  if (isatty(0)) {
    tcgetattr(0, &ts_show);
    memcpy(&ts_hide, &ts_show, sizeof(ts_show));
    ts_hide.c_lflag &= ~ECHO;
  }

  if (params->uname == NULL) {
    printf("username> ");
    fflush(stdout);
    getline(&(params->uname), &lsize, stdin);
    lsize = strlen(params->uname);
    params->uname[lsize - 1] = '\0'; /* get rid of '\n' */
    free(bufptr);
    bufptr = NULL;
  }

  if (params->pass == NULL) {
    // hides input letters
    if (isatty(0)) {
      tcsetattr(0, TCSANOW, &ts_hide);
    }
    do {
      if (params->pass != NULL) {
        printf("\nERROR: password do not match! Try again!\n");
        free(params->pass);
        params->pass = NULL;
      }
      if (bufptr != NULL) {
        free(bufptr);
        bufptr = NULL;
      }
      printf("password> ");
      getline(&(params->pass), &lsize, stdin);
      printf("\npassword (repeat)> ");
      getline(&(bufptr), &lsize, stdin);
    } while (strcmp(bufptr, params->pass));

    // shows input letters again
    if (isatty(0)) {
      tcsetattr(0, TCSANOW, &ts_show);
      putchar('\n');
    }

    printf("email> ");
    fflush(stdout);
    scanf("%s", email);

    lsize = strlen(params->pass);
    params->pass[lsize - 1] = '\0'; /* get rid of '\n' */
    free(bufptr);
    bufptr = NULL;
  }
}