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

void init_notification_modal_cb(w_cb_args_t *args) {
  w_cb_args_t d_args;
  w_app_t *app = args->app;
  w_dialogue_t *d = (w_dialogue_t *)app->active_widget;
  memcpy(&d_args, args, sizeof(w_cb_args_t));
  d_args.app = NULL;
  d_args.element = app->active_widget;
  w_dialogue_callback_default(&d_args);

  app->query_args->state = WAIT_CLIENT;
  d->needs_destroy = true;
}

w_dialogue_t *m_notification_init(w_app_t *app) {
  if (app == NULL)
    return NULL;

  w_group_el_init_t actions[] = {
      {.type = w_button, .label = "OK", .is_default = true, .val.num = 1},
      {.type = w_end}};

  w_dialogue_init(&(app->modal), "Notification", app->query_args->notification,
                &(app->coordinates));
  free(app->query_args->notification);
  app->query_args->notification = NULL;
  w_dialogue_t *d = &(app->modal);
  app->query_args->active_dialogue = d;

  d->w.callback = init_notification_modal_cb;
  d->g_content = NULL;
  d->g_action = w_group_init(&(d->win), &(d->w), actions, &(d->id_map),
                           horizontal, g_action);

  w_dialogue_init_active_id(d);

  return NULL;
}