#include "alert.h"
#include <ncursesw/ncurses.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <widget.h>

/* Short notifications. Prefer using this method to simply notify the client */

static app_t *app;

void alert_cb(callback_args_t *args) {
  //
}

void init_alert(app_t *_app) { app = _app; }

void alert(const char *message) { notification("Alert", dc_alert, message); }

void notification(const char *title, enum dc_color_scheme color,
                  const char *f_message, ...) {
  if (app == NULL)
    return;

  void *callback = app->active_callback;

  group_el_init_t actions[] = {
      {.type = w_button, .label = "OK", .is_default = true}, {.type = w_end}};

  dialogue_t *d = malloc(sizeof(dialogue_t));
  va_list args;
  va_start(args, f_message);

  vinit_dialogue(d, title, &(app->coordinates), f_message, &args);

  d->w.callback = alert_cb;
  d->g_content = NULL;
  d->g_action = init_group(&(d->win), &(d->w), actions, &(d->id_map),
                           horizontal, g_action);

  dialogue_init_active_id(d);
  d->color_scheme = color;

  draw_dialogue(d);
  wrefresh(d->win);
  int c = 0;

  while ((c = wgetch(d->win))) {
    if (c == '\n')
      break;
  }

  destroy_group(d->g_action);
  delwin(d->win);
  free(d);
  app->active_callback = callback;
  app_refresh(app);
  if (app->modal.win != NULL) {
    app->modal.needs_update = true;
  }
}