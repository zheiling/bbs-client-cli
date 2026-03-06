#include <stdint.h>
#include <string.h>
#include <ncursesw/ncurses.h>
#include <widget.h>
#include <widget_core.h>
#include "../connection.h"
#include "group.h"

typedef struct {
  button_t *element;
  enum w_type type;
} group_bt_t;

void asa_modal_dialogue_callback(callback_args_t *args) {
  callback_args_t d_args;
  app_t *app = args->app;
  dialogue_t *d = (dialogue_t *)app->active_widget;
  memcpy(&d_args, args, sizeof(callback_args_t));
  d_args.app = NULL;
  d_args.element = app->active_widget;
  input_t *in_ip = d->g_content->elements[0].element;
  input_t *in_port = d->g_content->elements[1].element;
  dialogue_default_callback(&d_args);
  if (d_args.resp_data.code == cbrp_val) {
    switch (d_args.resp_data.val.val.num) {
    case 1:
      get_ip_port(app->params, in_ip->value, in_port->value);
      connect_to_server(app);
      d->needs_destroy = true;
      app->query_args->state = S_WAIT_SERVER;
      break;
    case 2:
      destroy_app(app, 0);
    }
  }
}

dialogue_t *init_asa_modal(app_t *app) {
  if (app == NULL)
    return NULL;
  group_el_init_t content[] = {
      {.type = w_input, .label = "Address", .length = 15},
      {.type = w_input, .label = "Port", .length = 5},
      {.type = w_end}};

  group_el_init_t actions[] = {
      {.type = w_button, .label = "Connect", .is_default = true, .val.num = 1},
      {.type = w_button, .label = "Cancel", .is_default = false, .val.num = 2},
      {.type = w_end}};

  init_dialogue(&(app->modal), "Connect to server",
                "There is information needed", &(app->coordinates));
  dialogue_t *d = &(app->modal);

  d->w.callback = asa_modal_dialogue_callback;
  d->g_content = init_group(&(d->win), &(d->w), content,&(d->id_map), horizontal, g_content);
  d->g_action = init_group(&(d->win), &(d->w), actions,&(d->id_map), horizontal, g_action);

  dialogue_init_active_id(d);

  return NULL;
}