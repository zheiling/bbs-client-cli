#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <widget.h>

typedef struct {
  button_t *element;
  enum w_type type;
} group_bt_t;

void login_modal_option_cb(callback_args_t *args) {
  int32_t response;
  callback_args_t d_args;
  app_t *app = args->app;
  dialogue_t *d = (dialogue_t *)app->active_widget;
  memcpy(&d_args, args, sizeof(callback_args_t));
  d_args.app = NULL;
  d_args.resp_data = &response;
  d_args.element = app->active_widget;
  dialogue_default_callback(&d_args);
  if (response > -1) {
    switch (response) {
    case 0:
      d->needs_destroy = true;
      app->params->uname = malloc(sizeof "anonymous");
      strcpy(app->params->uname, "anonymous");
      write(app->params->sd, app->params->uname, sizeof "anonymous"-1);
      app->query_args->state = S_WAIT_SERVER;
      print_bars(app);
      break;
    case 1:
      d->needs_destroy = true;
      app->query_args->state = S_ASK_LOGIN_USER;
      break;
    case 2:
      d->needs_destroy = true;
      app->query_args->state = S_ASK_REGISTER;
      break;
    }
  }
}

dialogue_t *init_login_option_modal(app_t *app) {
  if (app == NULL)
    return NULL;
  group_el_init_t children[] = {{.type = w_button, .label = "Anonymous"},
                                {.type = w_button, .label = "User"},
                                {.type = w_button, .label = "Register"},
                                {.type = w_end}};

  init_dialogue(&(app->modal), "Login",
                "Select your login "
                "option",
                &(app->coordinates));
  app->modal.g_action =
      init_group(&(app->modal.win), &(app->modal.w), children,
                 horizontal);
  app->modal.w.callback = login_modal_option_cb;
  dialogue_init_active_id(&(app->modal));

  return NULL;
}