#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../app.h"
#include "../widget/dialogue.h"
#include "../widget/group.h"
#include "../widget/input.h"

typedef struct {
  button_t *element;
  enum w_type type;
} group_bt_t;

void init_login_credentials_modal_cb(callback_args_t *args) {
  int32_t response;
  callback_args_t d_args;
  app_t *app = args->app;
  dialogue_t *d = (dialogue_t *)args->widget;
  memcpy(&d_args, args, sizeof(callback_args_t));
  d_args.app = NULL;
  d_args.resp_data = &response;
  input_t *in_name = d->g_content->elements[0].element;
  input_t *in_pass = d->g_content->elements[1].element;
  dialogue_default_callback(&d_args);
  if (response > -1) {
    switch (response) {
    case 0:
      app->params->uname = malloc(in_name->value_len + 1);
      app->params->pass = malloc(in_pass->value_len + 1);
      strncpy(app->params->uname, in_name->value, in_name->value_len);
      app->params->uname[in_name->value_len] = 0;
      strncpy(app->params->pass, in_pass->value, in_pass->value_len);
      app->params->pass[in_pass->value_len] = 0;
      write(app->params->sd, app->params->uname, in_name->value_len);
      app->query_args->state = S_WAIT_SERVER;
      destroy_dialogue(d);
      break;
    case 1:
      destroy_app(app);
      exit(0);
    }
  }
}

dialogue_t *init_login_credentials_modal(app_t *app) {
  if (app == NULL)
    return NULL;
  group_el_init_t content[] = {
      {.type = w_input, .label = "Username", .length = 15},
      {.type = w_input, .label = "Password", .length = 15},
      {.type = w_end}};

  group_el_init_t actions[] = {
      {.type = w_button, .label = "Login", .is_default = 1},
      {.type = w_button, .label = "Cancel", .is_default = 0},
      {.type = w_end}};

  init_dialogue(&(app->modal), "Credentials", "There is information needed",
                &(app->coordinates));
  dialogue_t *d = &(app->modal);

  d->w.callback = init_login_credentials_modal_cb;
  d->g_content = init_group(&(d->win), &(d->w), content, horizontal);
  d->g_action = init_group(&(d->win), &(d->w), actions, horizontal);

  dialogue_init_active_id(d);

  return NULL;
}