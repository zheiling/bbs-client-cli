#include "group.h"
#include <ncursesw/ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <widget.h>
#include "../server.h"

typedef struct {
  w_button_t *element;
  enum w_type type;
} group_bt_t;

void login_modal_option_cb(w_cb_args_t *args) {
  w_cb_args_t d_args;
  w_app_t *app = args->app;
  w_dialogue_t *d = (w_dialogue_t *)app->active_widget;
  memcpy(&d_args, args, sizeof(w_cb_args_t));
  d_args.app = NULL;
  d_args.element = app->active_widget;
  w_dialogue_callback_default(&d_args);
  if (d_args.resp_data.code == cbrp_val) {
    switch (d_args.resp_data.val.val.num) {
    case 1:
      d->needs_destroy = true;
      app->params->uname = malloc(sizeof "anonymous");
      strcpy(app->params->uname, "anonymous");
      server_send_string(app->query_args, app->params->uname);
      app->query_args->state = S_WAIT_SERVER;
      w_app_draw_bars(app);
      break;
    case 2:
      d->needs_destroy = true;
      app->query_args->state = S_ASK_LOGIN_USER;
      break;
    case 3:
      d->needs_destroy = true;
      app->query_args->state = S_ASK_REGISTER;
      break;
    }
  }
}

w_dialogue_t *m_login_option_init(w_app_t *app) {
  if (app == NULL)
    return NULL;
  w_group_el_init_t children[] = {{.type = w_button, .label = "Anonymous", .val.num = 1},
                                {.type = w_button, .label = "User", .val.num = 2},
                                {.type = w_button, .label = "Register", .val.num = 3},
                                {.type = w_end}};

  w_dialogue_init(&(app->modal), "Login",
                "Select your login "
                "option",
                &(app->coordinates));
  app->modal.g_action = w_group_init(&(app->modal.win), &(app->modal.w), children,
                                   &(app->modal.id_map), horizontal, g_action);
  app->modal.w.callback = login_modal_option_cb;
  w_dialogue_init_active_id(&(app->modal));

  return NULL;
}