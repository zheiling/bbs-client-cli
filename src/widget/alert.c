#include "alert.h"
#include <ncursesw/ncurses.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <widget.h>

/* Short notifications. Prefer using this method to simply notify the client */

static w_app_t *app;

void alert_cb(w_cb_args_t *args) {
  //
}

void w_alert_init(w_app_t *_app) { app = _app; }

void w_alert(const char *message) { w_notification("Alert", dc_alert, message); }

void w_notification(const char *title, enum w_dialogue_color_scheme color,
                  const char *f_message, ...) {
  if (app == NULL)
    return;

  void *callback = app->active_callback;

  w_group_el_init_t actions[] = {
      {.type = w_button, .label = "OK", .is_default = true}, {.type = w_end}};

  w_dialogue_t *d = malloc(sizeof(w_dialogue_t));
  va_list args;
  va_start(args, f_message);

  w_dialogue_vinit(d, title, &(app->coordinates), f_message, &args);

  d->w.callback = alert_cb;
  d->g_content = NULL;
  d->g_action = w_group_init(&(d->win), &(d->w), actions, &(d->id_map),
                           horizontal, g_action);

  w_dialogue_init_active_id(d);
  d->color_scheme = color;

  w_dialogue_draw(d);
  wrefresh(d->win);
  int c = 0;

  while ((c = wgetch(app->win))) {
    if (c == '\n')
      break;
  }

  w_group_destroy(d->g_action);
  delwin(d->win);
  free(d);
  app->active_callback = callback;
  w_app_refresh(app);
  if (app->modal.win != NULL) {
    app->modal.needs_update = true;
  }
}