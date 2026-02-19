#include <ncurses.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <widget.h>

#include "alert.h"

typedef struct {
  button_t *element;
  enum w_type type;
} group_bt_t;

void init_register_modal_cb(callback_args_t *args) {
  char query[256];
  int32_t qlen = 0;
  int32_t response;
  callback_args_t d_args;
  app_t *app = args->app;
  dialogue_t *d = (dialogue_t *)app->active_widget;
  memcpy(&d_args, args, sizeof(callback_args_t));
  d_args.app = NULL;
  d_args.resp_data = &response;
  d_args.element = app->active_widget;
  input_t *in_name = d->g_content->elements[0].element;
  input_t *in_pass = d->g_content->elements[1].element;
  input_t *in_pass_r = d->g_content->elements[2].element;
  dialogue_default_callback(&d_args);
  if (response > -1) {
    switch (response) {
    case 0:
      if (strcmp(in_pass->value, in_pass_r->value)) {
        alert("Your passwords do not match");
        in_pass->value[0] = '\0';
        in_pass->value_len = 0;
        in_pass_r->value[0] = '\0';
        in_pass_r->value_len = 0;
        return;
      }
      app->params->uname = malloc(in_name->value_len + 1);
      app->params->pass = malloc(in_pass->value_len + 1);
      strncpy(app->params->uname, in_name->value, in_name->value_len);
      app->params->uname[in_name->value_len] = 0;
      strncpy(app->params->pass, in_pass->value, in_pass->value_len);
      app->params->pass[in_pass->value_len] = 0;
      sprintf(query, "register %s %s\n%n", app->params->uname, app->params->pass,
              &qlen);
      write(app->params->sd, query, qlen);
      app->query_args->state = S_WAIT_REGISTER_CONFIRMATION;
      break;
    case 1:
      app->query_args->state = S_ASK_LOGIN_TYPE;
      app->modal.needs_destroy = true;
    }
  }
}

dialogue_t *init_register_modal(app_t *app) {
  if (app == NULL)
    return NULL;
  group_el_init_t content[] = {
      {.type = w_input, .label = "Username", .length = 15},
      {.type = w_input,
       .label = "Password",
       .length = 15,
       .is_hidden_value = true},
      {.type = w_input,
       .label = "Password (repeat)",
       .length = 15,
       .is_hidden_value = true},
      {.type = w_end}};

  group_el_init_t actions[] = {
      {.type = w_button, .label = "Register", .is_default = true},
      {.type = w_button, .label = "Cancel", .is_default = false},
      {.type = w_end}};

  init_dialogue(&(app->modal), "Credentials", "There is information needed",
                &(app->coordinates));
  dialogue_t *d = &(app->modal);

  d->w.callback = init_register_modal_cb;
  d->g_content = init_group(&(d->win), &(d->w), content, horizontal);
  d->g_action = init_group(&(d->win), &(d->w), actions, horizontal);

  dialogue_init_active_id(d);

  return NULL;
}