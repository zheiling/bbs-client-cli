#include "modals/ask_server_addr.h"
#include "modals/download_pr.h"
#include "modals/login_credentials.h"
#include "modals/login_option.h"
#include "modals/notification.h"
#include "modals/server_message.h"
#include "modals/upload_dialogue.h"
#include "modals/upload_props_dialogue.h"
#include "widget/dialogue.h"
#include "main.h"

void app_draw_modal(app_t *app) {
  if (app->modal.needs_destroy ||
      (app->query_args->state == S_WAIT_SERVER && app->modal.is_initiated)) {
    destroy_dialogue(&(app->modal), app);
  }
  if (!app->modal.is_initiated) {
    switch (app->query_args->state) {
    case S_ASK_SEVER_IP:
      init_asa_modal(app);
      break;
    case S_ASK_LOGIN_TYPE:
      init_login_option_modal(app);
      break;
    case S_ASK_LOGIN_USER:
      init_login_credentials_modal(app);
      break;
    case S_PRINT_SERVER_MESSAGE:
      init_server_message_modal(app);
      break;
    case S_FILE_DOWNLOAD:
      init_dwn_pr_modal(app, false);
      break;
    case S_UPLOAD_FILE:
      init_dwn_pr_modal(app, true);
      break;
    case S_UPLOAD_FILE_SELECT:
      init_upload_dialogue_modal(app);
      break;
    case S_UPLOAD_PARAMS:
      init_upload_props_dialogue_modal(app);
      break;
    case WAIT_SERVER_INIT:
    case WAIT_SERVER:
    case WAIT_REGISTER:
    case WAIT_REGISTER_CONFIRMATION:
    case WAIT_CLIENT:
    case S_FILE_LIST:
    case S_FILE_SELECT:
    case S_UPLOAD_REQUESTED:
    case S_ASK_USER_BEFORE_LOGIN:
    case S_ERR:
    case S_WAIT_SERVER:
    case S_N_D:
    case S_NEXT_ACTION:
      if (app->query_args->notification != NULL) {
        init_notification_modal(app);
      } else {
        return;
      }
    }
    app->active_callback = app->modal.w.callback;
    app->active_win = app->modal.win;
    app->active_win_type = aw_modal;
    app->active_widget = &(app->modal);
  }
  draw_dialogue(&(app->modal));
}