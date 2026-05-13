/* #include "../file_processor.h"
#include "../windows/fs_file_list.h"
#include "group.h"
#include "widget_core.h"
#include <ncursesw/ncurses.h>
#include <stdint.h>
#include <string.h>
#include <widget.h>

void upload_dialogue_modal_cb(w_cb_args_t *args) {
  w_cb_args_t d_args;
  app_t *app = args->app;
  w_dialogue_t *d = (w_dialogue_t *)app->active_widget;
  memcpy(&d_args, args, sizeof(w_cb_args_t));
  d_args.app = NULL;
  d_args.element = app->active_widget;
  w_dialogue_callback_default(&d_args);
  w_lfl_ui_t *fui = (w_lfl_ui_t *)d->g_content->elements[0].element;
  if (d_args.resp_data.code == cbrp_val) {
    switch (d_args.resp_data.val.val.num) {
    case 0:
      app->query_args->state = S_WAIT_SERVER;
      d->needs_destroy = true;
      break;
    case 1:
      file_upload_open(fui->d_path, fui->current->name, app->query_args);
      d->needs_destroy = true;
      app->query_args->state = S_UPLOAD_PARAMS;
      break;
    case -2:
      d->needs_destroy = true;
      app->query_args->state = WAIT_CLIENT;
    }
  }
}

w_dialogue_t *m_upload_dialogue_init(app_t *app) {
  if (app == NULL)
    return NULL;
  w_group_el_init_t content[] = {
      // {.type = w_input, .label = "File description", .length = 30},
      {.type = w_box, .label = "File", .length = 30},
      {.type = w_end}};

  w_group_el_init_t actions[] = {
      {.type = w_button, .label = "Cancel", .is_default = false, .val.num = 0},
      {.type = w_end}};

  app->modal.w.parent_win = &app->win;
  w_dialogue_init(&(app->modal), "Upload new file", "Enter essential data",
                  &(app->coordinates));
  w_dialogue_t *d = &(app->modal);

  d->w.callback = upload_dialogue_modal_cb;
  d->g_content = w_group_init(&(d->win), &(d->w), content, &(d->id_map),
                              horizontal, g_content);
  d->g_action = w_group_init(&(d->win), &(d->w), actions, &(d->id_map),
                             horizontal, g_action);

  app->query_args->active_dialogue = d;

  w_dialogue_init_active_id(d);

  return NULL;
} */