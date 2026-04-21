/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#include "main_window.h"
#include "fs_file_list.h"
#include "file_list.h"

void main_window_set(w_app_t *app, enum main_window_type type) {
  if (app->main_ui.ui != NULL) {
    main_window_destroy(app);
  }
  app->main_ui.type = type;
  switch (type) {
  case mw_fl_server:
    w_fl_init(app);
    app->query_args->state = S_FILE_LIST;
    break;
  case mw_fl_local:
    w_lfl_init_win(app);
    break;
  case mw_f_desc:
    break;
  default:
    break;
  }
}

void main_window_draw(w_app_t *app) {
  switch (app->main_ui.type) {
  case mw_fl_server:
    w_fl_draw((w_ui_file_list_t *)app->main_ui.ui);
    break;
  case mw_fl_local:
    w_lfl_draw((w_lfl_ui_t *)app->main_ui.ui);
    break;
  case mw_f_desc:
    break;
  }
  w_app_draw_bbar(app);
}

void main_window_destroy(w_app_t *app) {
  switch (app->main_ui.type) {
  case mw_fl_server:
    w_fl_destroy((w_ui_file_list_t **)&(app->main_ui.ui));
    break;
  case mw_fl_local:
    w_lfl_destroy((w_lfl_ui_t **)&(app->main_ui.ui));
  case mw_f_desc:
    break;
  }
  MAIN_UI_RESET(app);
}