#include "alert.h"
#include "dialogue.h"
#include <core/app.h>
#include <ncursesw/ncurses.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <widget.h>

/* Short notifications. Prefer using this method to simply notify the client */

static app_t *app;

void alert_cb(w_cb_args_t *args) {
  //
}

void w_alert_init(app_t *_app) { app = _app; }

void w_alert(const char *message) {
  w_notification("Alert", dc_alert, message);
}

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
  app_refresh(app);
  if (app->modal.win != NULL) {
    app->modal.needs_update = true;
  }
}

int w_bool_callback(app_t *app, w_dialogue_t *dialogue, int32_t key) {
  w_cb_args_t args;
  args.app = app;
  args.active_el = dialogue;
  args.element = dialogue;
  int _key = key;
  args.data = (void *)&_key;
  w_dialogue_callback_default(&args);
  if (args.resp_data.code == cbrp_val) {
    switch (args.resp_data.val.val.num) {
    case 1:
      return 1;
    case 2:
      return 2;
    default:
      return -2;
    }
  }
  return 0;
}

int w_bool_ask(const char *title, const char *text_y_btn,
               const char *text_n_btn, bool yes_default, const char *f_message,
               ...) {

  va_list args;
  va_start(args, f_message);

  if (app == NULL)
    return -1;

  void *callback = app->active_callback;

  w_group_el_init_t actions[] = {{.type = w_button, .val.num = 1}, /* YES */
                                 {.type = w_button, .val.num = 2}, /* NO */
                                 {.type = w_end}};

  strcpy(actions[0].label, text_y_btn);
  strcpy(actions[1].label, text_n_btn);

  w_dialogue_t *d = malloc(sizeof(w_dialogue_t));

  w_dialogue_vinit(d, title, &(app->coordinates), f_message, &args);

  d->w.callback = alert_cb;
  d->g_content = NULL;
  d->g_action = w_group_init(&(d->win), &(d->w), actions, &(d->id_map),
                             horizontal, g_action);

  if (yes_default) {
    d->active_el = d->g_action->elements;
  } else {
    d->active_el = d->g_action->elements + 1;
  }

  w_dialogue_draw(d);
  wrefresh(d->win);
  int c = 0;

  bool response = false;

  while ((c = wgetch(app->win))) {
    int ret = w_bool_callback(app, d, c);
    switch (ret) {
    case 1:
      response = true;
      goto break_while;
    case 2:
      goto break_while;
    }
    w_dialogue_draw(d);
    wrefresh(d->win);
  }

break_while:

  w_group_destroy(d->g_action);
  delwin(d->win);
  free(d);
  app->active_callback = callback;
  app_refresh(app);
  if (app->modal.win != NULL) {
    app->modal.needs_update = true;
  }

  return response;
}