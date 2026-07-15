/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#include <arpa/inet.h>
#include <modals.h>
#include <ncursesw/ncurses.h>
#include <netinet/in.h>
#include <stdint.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <termios.h>
#include <unistd.h>
#include <widget.h>
#include <windows.h>

#include "../app.h"
#include "alert.h"
#include "connection.h"
// #include "file_list.h"
#include "main.h"
#include "query.h"

int32_t m_id = 0;

int32_t process_user_input(app_t *app, w_cb_args_t *d_args);

int main(int argc, char **argv) {
  app_t app;
  params_t params;
  query_args_t q_args;

  app_init_nc();

  app.params = &params;
  app_init(&app);
  init_params(&params);
  analyze_args(argc, argv, &params);

  /* init alert */
  w_alert_init(&app);

  init_query_args(&q_args, app.params);
  app.query_args = &q_args;

  main_window_set(&app, mw_fl_server);
  /* init client to connect to the server */
  app.params->sd = init_client();

  if (params.addr == 0) {
    app.query_args->state = S_ASK_SEVER_IP;
  } else {
    connect_to_server(&app);
  }

  app.query_args->main_ui = &(app.main_ui);

  query_loop(&app);
  clear_params(&params);
  app_destroy(&app, 0);

  return OK;
}

int32_t process_user_input(app_t *app, w_cb_args_t *d_args) {
  int32_t c;
  w_ui_file_list_t *fui = (w_ui_file_list_t *)app->query_args->main_ui->ui;
  c = wgetch(app->win);
  switch (c) {
  case KEY_F(9):
    app_destroy(app, 0);
    return OK;
  case 'U':
  case 'u':
    if (!app->modal.is_initiated && !fui->active_search &&
        app->main_ui.type != mw_fl_local) {
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
      w_fl_draw(fui);
      return OK;
    }
  default:
    d_args->data = (void *)&c;
    app->active_callback(d_args);
    break;
  }
  return OK;
}