#include "../app.h"
#include "../widget/dialogue.h"
#include "../widget/group.h"
#include <ncurses.h>
#include <stdint.h>
#include <string.h>

typedef struct {
  button_t *element;
  enum w_type type;
} group_bt_t;

void login_modal_group_callback(callback_args_t *args) {
  int32_t response;
  callback_args_t d_args;
  app_t *app = args->app;
  dialogue_t *d = (dialogue_t *)args->widget;
  memcpy(&d_args, args, sizeof(callback_args_t));
  d_args.app = NULL;
  d_args.resp_data = &response;
  dialogue_default_callback(&d_args);
  if (response > -1) {
    switch (response) {
    case 0:
      break;
    case 1:
      // exit(0);
      break;
    }
  }
}

dialogue_t *init_login_modal(app_t *app) {
  if (app == NULL)
    return NULL;
  group_el_init_t children[] = {{.type = w_button, .label = "Anonymous"},
                                {.type = w_button, .label = "User"},
                                {.type = w_button, .label = "Register"},
                                {.type = w_end}};

  init_dialogue(&(app->modal.dialogue), "Login",
                "Select your login "
                "option",
                &(app->coordinates));
  app->modal.dialogue.g_content =
      init_group(&(app->modal.dialogue.win), &(app->modal.dialogue.w), children,
                 horizontal);
  app->modal.dialogue.w.callback = login_modal_group_callback;

  return NULL;
}