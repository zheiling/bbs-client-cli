#include "../app.h"
#include "../widget/dialogue.h"
#include "../widget/group.h"
#include <ncurses.h>
#include <stdint.h>
#include <stdio.h>

void dwn_pr_modal_cb(callback_args_t *args) {
}

dialogue_t *init_dwn_pr_modal(app_t *app) {
  if (app == NULL)
    return NULL;
  group_el_init_t content[] = {{.type = w_progress, .label = "", .length = 30}, /* length is ignored */
                               {.type = w_end}};

  char text[256];
  sprintf(text, "Downloading file: %s", app->file_args->f_selected.name);
  group_el_init_t actions[] = {
      {.type = w_button, .label = "Cancel", .is_default = 0}, {.type = w_end}};

  app->modal.w.parent_win = &app->win;
  init_dialogue(&(app->modal), "Download", text, &(app->coordinates));
  dialogue_t *d = &(app->modal);

  d->w.callback = dwn_pr_modal_cb;
  d->g_content = init_group(&(d->win), &(d->w), content, horizontal);
  d->g_action = init_group(&(d->win), &(d->w), actions, horizontal);

  app->query_args->progress_bar = d->g_content->elements[0].element;
  app->query_args->active_dialogue = d;

  dialogue_init_active_id(d);

  return NULL;
}