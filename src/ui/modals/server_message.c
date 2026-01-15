#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../app.h"
#include "../widget/dialogue.h"
#include "../widget/group.h"

typedef struct {
  button_t *element;
  enum w_type type;
} group_bt_t;

void server_message_modal_cb(callback_args_t *args) {
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
      destroy_dialogue(d, app);
      /* TODO: macros */
      free(app->query_args->server_message.text);
      app->query_args->server_message.capacity = 0;
      app->query_args->server_message.size = 0;
      app->query_args->state = S_NEXT_ACTION;
      break;
    }
  }
}

void *init_server_message_modal(app_t *app) {
  if (app == NULL)
    return NULL;
  group_el_init_t children[] = {
      {.type = w_button, .label = "OK", .is_default = 1}, {.type = w_end}};

  init_dialogue(&(app->modal), "Server response",
                app->query_args->server_message.text, &(app->coordinates));
  app->modal.g_action =
      init_group(&(app->modal.win), &(app->modal.w), children, horizontal);
  app->modal.w.callback = server_message_modal_cb;
  dialogue_init_active_id(&(app->modal));

  return NULL;
}