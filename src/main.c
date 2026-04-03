/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#include <arpa/inet.h>
#include <modals.h>
#include <ncursesw/ncurses.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <termios.h>
#include <unistd.h>
#include <widget.h>

#include "app.h"
#include "connection.h"
#include "file_list.h"
#include "fs_file_list.h"
#include "main.h"
#include "query.h"

uint32_t m_id = 0;

int32_t process_user_input(app_t *app, callback_args_t *d_args);
void main_window_set(app_t *app, enum main_window_type type);
void main_window_draw(app_t *app);
void main_window_delete(app_t *app);

int main(int argc, char **argv) {
  app_t *app;
  params_t params;
  query_args_t *q_args = malloc(sizeof(query_args_t));

  app = calloc(1, sizeof(app_t));

  init_nc();

  app = init_app();
  init_params(&params);
  app->params = &params;
  analyze_args(argc, argv, &params);

  /* init alert */
  init_alert(app);

  wrefresh(app->left_win);
  wrefresh(app->right_win);

  init_query_args(q_args, app->params);
  q_args->buf = malloc(INBUFSIZE);
  app->query_args = q_args;

  main_window_set(app, mw_fl_server);
  main_window_draw(app);

  /* init client to connect to the server */
  app->params->sd = init_client();

  if (params.addr == 0) {
    app->query_args->state = S_ASK_SEVER_IP;
  } else {
    connect_to_server(app);
    app->query_args->state = S_WAIT_SERVER;
  }

  app->query_args->sd = app->params->sd;
  app->query_args->main_ui = &(app->main_ui);

  query_loop(app);
  clear_params(&params);
  destroy_app(app, 0);

  return OK;
}

int32_t process_user_input(app_t *app, callback_args_t *d_args) {
  int32_t c;
  ui_file_list_t *fui = (ui_file_list_t *)app->query_args->main_ui->ui;
  c = wgetch(app->win);
  switch (c) {
  case KEY_F(9):
    destroy_app(app, 0);
    return OK;
  case 'U':
  case 'u':
    if (!app->modal.is_initiated && !fui->active_search && app->main_ui.type != mw_fl_local) {
      /* app->query_args->state = S_UPLOAD_FILE_SELECT; */
      main_window_set(app, mw_fl_local);
      main_window_draw(app);
      d_args->element = app->main_ui.ui;
      break;
    }
  case 's':
  case 'S':
    if (!app->modal.is_initiated && !fui->active_search) {
      fui->active_search = true;
      draw_file_list(fui);
      return OK;
    }
  default:
    d_args->data = (void *)&c;
    app->active_callback(d_args);
    break;
  }
  return OK;
}

void main_window_set(app_t *app, enum main_window_type type) {
  if (app->main_ui.ui != NULL) {
    main_window_delete(app);
  }
  app->main_ui.type = type;
  switch (type) {
  case mw_fl_server:
    app->main_ui.ui = init_file_list(&(app->left_win), &(app->right_win));
    app->active_callback = file_list_cb;
    break;
  case mw_fl_local:
    app->main_ui.ui =
        init_fs_file_list_win(&(app->left_win), &(app->right_win));
    app->active_callback = fs_file_list_cb;
    break;
  case mw_f_desc:
    break;
  default:
    break;
  }
}

void main_window_draw(app_t *app) {
  switch (app->main_ui.type) {
  case mw_fl_server:
    draw_file_list((ui_file_list_t *)app->main_ui.ui);
    break;
  case mw_fl_local:
    draw_fs_file_list((ui_fs_file_list_t *)app->main_ui.ui);
    break;
  case mw_f_desc:
    break;
  }
}

void main_window_delete(app_t *app) {
  switch (app->main_ui.type) {
  case mw_fl_server:
    delete_file_list((ui_file_list_t **)&(app->main_ui.ui));
    break;
  case mw_fl_local:
  /* TODO: delete fs_file_list */
  case mw_f_desc:
    break;
  }
}