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

void init_login_credentials_modal_cb(w_cb_args_t *args) {
  w_cb_args_t d_args;
  w_app_t *app = args->app;
  w_dialogue_t *d = (w_dialogue_t *)app->active_widget;
  memcpy(&d_args, args, sizeof(w_cb_args_t));
  d_args.app = NULL;
  d_args.element = app->active_widget;
  w_input_t *in_name = d->g_content->elements[0].element;
  w_input_t *in_pass = d->g_content->elements[1].element;
  w_dialogue_callback_default(&d_args);
  if (d_args.resp_data.code == cbrp_val) {
    switch (d_args.resp_data.val.val.num) {
    case 1:
      app->params->uname = malloc(in_name->value_len + 1);
      app->params->pass = malloc(in_pass->value_len + 1);
      strncpy(app->params->uname, in_name->value, in_name->value_len);
      app->params->uname[in_name->value_len] = 0;
      strncpy(app->params->pass, in_pass->value, in_pass->value_len);
      app->params->pass[in_pass->value_len] = 0;
      server_send_string(app->query_args, app->params->uname);
      app->query_args->state = S_WAIT_SERVER;
      d->needs_destroy = true;
      w_app_draw_bars(app);
      break;
    case 2:
      app->query_args->state = S_ASK_LOGIN_TYPE;
      app->modal.needs_destroy = true;
    }
  }
}

w_dialogue_t *m_login_credentials_init(w_app_t *app) {
  if (app == NULL)
    return NULL;
  w_group_el_init_t content[] = {
      {.type = w_input, .label = "Username", .length = 15},
      {.type = w_input,
       .label = "Password",
       .length = 15,
       .is_hidden_value = true},
      {.type = w_end}};

  w_group_el_init_t actions[] = {
      {.type = w_button, .label = "Login", .is_default = true, .val.num = 1},
      {.type = w_button, .label = "Cancel", .is_default = false, .val.num = 2},
      {.type = w_end}};

  w_dialogue_init(&(app->modal), "Login", "Your login credentials",
                &(app->coordinates));
  w_dialogue_t *d = &(app->modal);

  d->w.callback = init_login_credentials_modal_cb;
  d->g_content = w_group_init(&(d->win), &(d->w), content, &(d->id_map), horizontal, g_content);
  d->g_action = w_group_init(&(d->win), &(d->w), actions, &(d->id_map), horizontal, g_action);

  w_dialogue_init_active_id(d);

  return NULL;
}