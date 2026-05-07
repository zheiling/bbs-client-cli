/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#include <modals.h>
#include <widget.h>
#include "main.h"

static void (*active_callback_backup)(w_cb_args_t *) = NULL;

void app_draw_modal(app_t *app) {
  if (app->modal.needs_destroy ||
      (app->query_args->state == S_WAIT_SERVER && app->modal.is_initiated)) {
    w_dialogue_destroy(&(app->modal), app);
    app->active_callback = active_callback_backup;
  }
  if (!app->modal.is_initiated) {
    active_callback_backup = app->active_callback;
    switch (app->query_args->state) {
    case S_ASK_SEVER_IP:
      m_asa_init(app);
      break;
    case S_ASK_LOGIN_TYPE:
      m_login_option_init(app);
      break;
    case S_ASK_LOGIN_USER:
      m_login_credentials_init(app);
      break;
    case S_PRINT_SERVER_MESSAGE:
      m_server_message_init(app);
      break;
    case S_FILE_DOWNLOAD:
      m_load_pr_init(app, false);
      break;
    case S_UPLOAD_FILE:
      m_load_pr_init(app, true);
      break;
    case S_ASK_REGISTER:
      m_register_init(app);
      break;
    case S_UPLOAD_PARAMS:
      m_upload_props_dialogue_init(app);
      break;
    default:
      if (app->query_args->notification != NULL) {
        m_notification_init(app);
      } else {
        return;
      }
    }
    app->active_callback = app->modal.w.callback;
    app->active_widget = &(app->modal);
  }
  w_dialogue_draw(&(app->modal));
}