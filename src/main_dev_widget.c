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
#include <windows.h>

#include "../app.h"
#include "alert.h"
#include "connection.h"
#include "main.h"
#include "query.h"

int32_t m_id = 0;


void mini_loop(app_t *app) {
  w_cb_args_t d_args = {
      .app = app, .element = NULL, .data = NULL, .resp_data.code = cbrc_none};

  fd_set readfds;

  while (true) {
    app_draw_modal(app);
    main_window_draw(app);
    app_refresh(app);
  
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    int maxfd = STDIN_FILENO;
  
    int sr = select(maxfd + 1, &readfds, NULL, NULL, NULL);
  
    if (sr == -1) {
      /* perror("select"); */
      exit(3);
    }
  
    if (app->main_ui.cb_b_press != NULL && app->modal.is_initiated == false) {
      if (ERR == app->main_ui.cb_b_press(app, &d_args)) {
        w_alert("error while processing user input");
      }
    }
  }

}

int main(void) {
  app_t app;
  file_args_t fargs;
  query_args_t q_args;
  params_t params;
  p_file_t file;

  app.params = &params;
  app.query_args = &q_args;
  app.file_args = &fargs;
  q_args.file = &file;
  app.query_args->main_ui = &(app.main_ui);

  file.name = "Test file #1";
  params.is_connected = false;
  app.modal.is_initiated = false;

  app_init_nc();
  app_init(&app);
  init_params(&params);
  init_query_args(&q_args, app.params);
  w_alert_init(&app);
  app_init(&app);
  main_window_set(&app, mw_f_desc);
  mini_loop(&app);
  exit(0);
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