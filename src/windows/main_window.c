/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#include "main_window.h"
#include "file_list.h"
#include "fs_file_list.h"
#include "text_edit.h"

void main_window_set(app_t *app, enum main_window_type type) {
  if (app->main_ui.ui != NULL) {
    main_window_destroy(app);
  }
  if (app->modal.is_initiated) {
    w_dialogue_destroy(&(app->modal), app);
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
    w_te_init_win(app);
    break;
  default:
    break;
  }
  app_draw_bars(app);
}

void main_window_draw(app_t *app) {
  switch (app->main_ui.type) {
  case mw_fl_server:
    w_fl_draw((w_ui_file_list_t *)app->main_ui.ui);
    break;
  case mw_fl_local:
    w_lfl_draw((w_lfl_ui_t *)app->main_ui.ui);
    break;
  case mw_f_desc:
    w_te_draw((w_te_ui_t *)app->main_ui.ui);
    break;
  }
  app_draw_bbar(app);
}

void main_window_destroy(app_t *app) {
  switch (app->main_ui.type) {
  case mw_fl_server:
    w_fl_destroy((w_ui_file_list_t **)&(app->main_ui.ui));
    break;
  case mw_fl_local:
    w_lfl_destroy((w_lfl_ui_t **)&(app->main_ui.ui));
    break;
  case mw_f_desc:
    w_te_destroy((w_te_ui_t **)&(app->main_ui.ui));
    break;
  }
  MAIN_UI_RESET(app);
}