#include "group.h"
#include <ncursesw/ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <widget.h>

typedef struct {
  w_button_t *element;
  enum w_type type;
} group_bt_t;

void server_message_modal_cb(w_cb_args_t *args) {
  w_cb_args_t d_args;
  app_t *app = args->app;
  w_dialogue_t *d = (w_dialogue_t *)app->active_widget;
  memcpy(&d_args, args, sizeof(w_cb_args_t));
  /* d_args.app = NULL; */
  d_args.element = app->active_widget;
  w_dialogue_callback_default(&d_args);
  if (d_args.resp_data.code == cbrp_val) {
    switch (d_args.resp_data.val.val.num) {
    case 1:
      d->needs_destroy = true;
      /* TODO: macros */
      free(app->query_args->server_message.text);
      app->query_args->server_message.text = NULL;
      app->query_args->server_message.capacity = 0;
      app->query_args->server_message.size = 0;
      app->query_args->state = S_NEXT_ACTION;
      break;
    }
  }
}

void *m_server_message_init(app_t *app) {
  if (app == NULL)
    return NULL;
  w_group_el_init_t children[] = {
      {.type = w_button, .label = "OK", .is_default = true, .val.num = 1}, {.type = w_end}};

  w_dialogue_init(&(app->modal), "Server response",
                app->query_args->server_message.text, &(app->coordinates));
  app->modal.g_action =
      w_group_init(&(app->modal.win), &(app->modal.w), children, &(app->modal.id_map), horizontal, g_action);
  app->modal.w.callback = server_message_modal_cb;
  w_dialogue_init_active_id(&(app->modal));

  return NULL;
}