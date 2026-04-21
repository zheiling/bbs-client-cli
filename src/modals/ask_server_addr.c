#include <stdint.h>
#include <string.h>
#include <ncursesw/ncurses.h>
#include <widget.h>
#include <widget_core.h>
#include "../connection.h"
#include "group.h"

typedef struct {
  w_button_t *element;
  enum w_type type;
} group_bt_t;

void asa_modal_dialogue_callback(w_cb_args_t *args) {
  w_cb_args_t d_args;
  app_t *app = args->app;
  w_dialogue_t *d = (w_dialogue_t *)app->active_widget;
  memcpy(&d_args, args, sizeof(w_cb_args_t));
  d_args.app = NULL;
  d_args.element = app->active_widget;
  w_input_t *in_ip = d->g_content->elements[0].element;
  w_input_t *in_port = d->g_content->elements[1].element;
  w_dialogue_callback_default(&d_args);
  if (d_args.resp_data.code == cbrp_val) {
    switch (d_args.resp_data.val.val.num) {
    case 1:
      get_ip_port(app->params, in_ip->value, in_port->value);
      connect_to_server(app);
      d->needs_destroy = true;
      app->query_args->state = S_WAIT_SERVER;
      break;
    case 2:
      app_destroy(app, 0);
    }
  }
}

w_dialogue_t *m_asa_init(app_t *app) {
  if (app == NULL)
    return NULL;
  w_group_el_init_t content[] = {
      {.type = w_input, .label = "Address", .length = 15},
      {.type = w_input, .label = "Port", .length = 5},
      {.type = w_end}};

  w_group_el_init_t actions[] = {
      {.type = w_button, .label = "Connect", .is_default = true, .val.num = 1},
      {.type = w_button, .label = "Cancel", .is_default = false, .val.num = 2},
      {.type = w_end}};

  w_dialogue_init(&(app->modal), "Connect to server",
                "There is information needed", &(app->coordinates));
  w_dialogue_t *d = &(app->modal);

  d->w.callback = asa_modal_dialogue_callback;
  d->g_content = w_group_init(&(d->win), &(d->w), content,&(d->id_map), horizontal, g_content);
  d->g_action = w_group_init(&(d->win), &(d->w), actions,&(d->id_map), horizontal, g_action);

  w_dialogue_init_active_id(d);

  return NULL;
}