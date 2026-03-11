#include <modals.h>
#include <widget.h>
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
    case S_ASK_REGISTER:
      init_register_modal(app);
      break;
    case S_UPLOAD_PARAMS:
      init_upload_props_dialogue_modal(app);
      break;
    default:
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