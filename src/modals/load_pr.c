#include "group.h"
#include <ncursesw/ncurses.h>
#include <stdint.h>
#include <stdio.h>
#include <widget.h>

void dwn_pr_modal_cb(w_cb_args_t *args) {
  /* only cancel action */
  int32_t key = *((int32_t *)args->data);
  if (key != '\n') {
    return;
  }
  app_t *app = args->app;
  app->query_args->file->signal = sig_cancel;
}

w_dialogue_t *m_load_pr_init(app_t *app, bool is_upload) {
  if (app == NULL)
    return NULL;
  w_group_el_init_t content[] = {
      {.type = w_progress, .label = "", .length = 30}, /* length is ignored */
      {.type = w_end}};

  char text[256];
  sprintf(text, "%s file: %s", is_upload ? "Uploading" : "Downloading",
          is_upload ? app->query_args->file->name
                    : app->file_args->f_selected.name);
  w_group_el_init_t actions[] = {
      {.type = w_button, .label = "Cancel", .is_default = true, .val.num = 1},
      {.type = w_end}};

  app->modal.w.parent_win = &app->win;
  w_dialogue_init(&(app->modal), is_upload ? "Upload" : "Download", text,
                  &(app->coordinates));
  w_dialogue_t *d = &(app->modal);

  d->w.callback = dwn_pr_modal_cb;
  d->g_content = w_group_init(&(d->win), &(d->w), content, &(d->id_map),
                              horizontal, g_content);
  d->g_action = w_group_init(&(d->win), &(d->w), actions, &(d->id_map),
                             horizontal, g_action);

  app->query_args->progress_bar = d->g_content->elements[0].element;
  app->query_args->active_dialogue = d;

  w_dialogue_init_active_id(d);

  return NULL;
}