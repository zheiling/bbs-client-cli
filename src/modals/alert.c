#include "app.h"
#include "dialogue.h"
#include "widget_core.h"
#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>

/* Short notifications. Prefer using this method to simply notify the client */

static app_t *app;
void notification(const char *title, const char *message,
                  enum d_color_scheme color);

void alert_cb(callback_args_t *args) {
  //
}

void init_alert(app_t *_app) { app = _app; }

void alert(const char *message) { notification("Alert", message, dc_alert); }

void notification(const char *title, const char *message,
                  enum d_color_scheme color) {
  if (app == NULL)
    return;

  group_el_init_t actions[] = {
      {.type = w_button, .label = "OK", .is_default = 1}, {.type = w_end}};

  dialogue_t *d = malloc(sizeof(dialogue_t));

  init_dialogue(d, title, message, &(app->coordinates));

  d->w.callback = alert_cb;
  d->g_content = NULL;
  d->g_action = init_group(&(d->win), &(d->w), actions, horizontal);

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
  app_refresh(app);
  if (app->modal.win != NULL) {
    app->modal.needs_update = true;
  }
}