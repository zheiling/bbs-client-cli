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

void init_notification_modal_cb(callback_args_t *args) {
  int32_t response;
  callback_args_t d_args;
  app_t *app = args->app;
  dialogue_t *d = (dialogue_t *)app->active_widget;
  memcpy(&d_args, args, sizeof(callback_args_t));
  d_args.app = NULL;
  d_args.resp_data = &response;
  d_args.element = app->active_widget;
  dialogue_default_callback(&d_args);

  app->query_args->state = WAIT_CLIENT;
  destroy_dialogue(d, app);
}

dialogue_t *init_notification_modal(app_t *app) {
  if (app == NULL)
    return NULL;

  group_el_init_t actions[] = {
      {.type = w_button, .label = "OK", .is_default = 1}, {.type = w_end}};

  init_dialogue(&(app->modal), "Notification", app->query_args->notification,
                &(app->coordinates));
  free(app->query_args->notification);
  app->query_args->notification = NULL;
  dialogue_t *d = &(app->modal);
  app->query_args->active_dialogue = d;

  d->w.callback = init_notification_modal_cb;
  d->g_content = NULL;
  d->g_action = init_group(&(d->win), &(d->w), actions, horizontal);

  dialogue_init_active_id(d);

  return NULL;
}