#include "app.h"
#include "dialogue.h"
#include "widget_core.h"
#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>

void init_alert_cb(callback_args_t *args) {
  //
}

void alert(app_t *app, const char *message) {
  if (app == NULL)
    return;

  group_el_init_t actions[] = {
      {.type = w_button, .label = "OK", .is_default = 1}, {.type = w_end}};

  dialogue_t *d = malloc(sizeof(dialogue_t));

  init_dialogue(d, "Alert", message, &(app->coordinates));

  d->w.callback = init_alert_cb;
  d->g_content = NULL;
  d->g_action = init_group(&(d->win), &(d->w), actions, horizontal);

  dialogue_init_active_id(d);
  d->color_scheme = dc_alert;

  draw_dialogue(d);
  wrefresh(d->win);
  int c = 0;

  while ((c = wgetch(d->win))) {
    if (c == '\n') break;
  }
  
  destroy_group(d->g_action);
  delwin(d->win);
  free(d);
  app_refresh(app);
}