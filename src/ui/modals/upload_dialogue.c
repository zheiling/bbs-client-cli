#include "../app.h"
#include "../widget/dialogue.h"
#include "../widget/fs_file_list.h"
#include "../widget/group.h"
#include "../../file_processor.h"
#include <ncurses.h>
#include <stdint.h>
#include <string.h>

void upload_dialogue_modal_cb(callback_args_t *args) {
  int32_t response = -1;
  callback_args_t d_args;
  app_t *app = args->app;
  dialogue_t *d = (dialogue_t *)app->active_widget;
  memcpy(&d_args, args, sizeof(callback_args_t));
  d_args.app = NULL;
  d_args.resp_data = &response;
  d_args.element = app->active_widget;
  dialogue_default_callback(&d_args);
  ui_fs_file_list_t *fui =
      (ui_fs_file_list_t *)d->g_content->elements[0].element;
  if (response > -1) {
    switch (response) {
    case 0:
      app->query_args->state = S_WAIT_SERVER;
      destroy_dialogue(d, app);
      break;
    case 1:
      file_upload_open(fui->d_path, fui->current->name, app->query_args);
      destroy_dialogue(d, app);
      app->query_args->state = S_UPLOAD_PARAMS;
      break;
    }
  }
}

dialogue_t *init_upload_dialogue_modal(app_t *app) {
  if (app == NULL)
    return NULL;
  group_el_init_t content[] = {
      // {.type = w_input, .label = "File description", .length = 30},
      {.type = w_fs_file_list, .label = "File", .length = 30},
      {.type = w_end}};

  group_el_init_t actions[] = {
      {.type = w_button, .label = "Cancel", .is_default = 0}, {.type = w_end}};

  app->modal.w.parent_win = &app->win;
  init_dialogue(&(app->modal), "Upload new file", "Enter essential data",
                &(app->coordinates));
  dialogue_t *d = &(app->modal);

  d->w.callback = upload_dialogue_modal_cb;
  d->g_content = init_group(&(d->win), &(d->w), content, horizontal);
  d->g_action = init_group(&(d->win), &(d->w), actions, horizontal);

  app->query_args->active_dialogue = d;

  dialogue_init_active_id(d);

  return NULL;
}