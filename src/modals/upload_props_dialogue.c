#include "checkbox.h"
#include "group.h"
#include "widget_core.h"
#include <ncursesw/ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utils.h>
#include <widget.h>

#include "../server.h"

void upload_props_dialogue_modal_cb(w_cb_args_t *args) {
  w_cb_args_t d_args;
  w_app_t *app = args->app;
  w_dialogue_t *d = (w_dialogue_t *)app->active_widget;
  memcpy(&d_args, args, sizeof(w_cb_args_t));
  d_args.app = NULL;
  d_args.element = app->active_widget;
  w_dialogue_callback_default(&d_args);
  int32_t query_len = 0;
  if (d_args.resp_data.code == cbrp_val) {
    w_checkbox_t *reg_cbx = (w_checkbox_t *)d->g_content->elements[0].element;
    w_checkbox_t *anon_cbx = (w_checkbox_t *)d->g_content->elements[1].element;
    w_checkbox_t *comp_cbx = (w_checkbox_t *)d->g_content->elements[2].element;

    int32_t privileges = 0;
    if (anon_cbx->value == true)
      privileges |= 1;
    if (reg_cbx->value == true)
      privileges |= 2;
    if (comp_cbx->value == true)
      privileges |= 4;

    switch (d_args.resp_data.val.val.num) {
    case 1:
      server_send_string(app->query_args, "file upload \"%s\" %zu %d\n",
                         app->query_args->file->name,
                         app->query_args->file->size, privileges);
      /* app->query_args->file->description =
          malloc(desc_input->value_len + 9); reserve space for \n:END:\n */
      app->query_args->file->description =
          malloc(sizeof("[Empty description]\n:END:\n"));
      strcpy(app->query_args->file->description,
             "[Empty description]\n:END:\n");
      d->needs_destroy = true;
      app->query_args->state = S_UPLOAD_REQUESTED;
      break;
    case 2:
      FREE_MLC(app->query_args->file->name);
      FREE_MLC(app->query_args->file->path)
      FREE_MLC(app->query_args->file);
      app->query_args->state = S_WAIT_SERVER;
      break;
    }
  }
}

w_dialogue_t *m_upload_props_dialogue_init(w_app_t *app) {
  if (app == NULL)
    return NULL;
  w_group_el_init_t content[] = {{.type = w_checkbox,
                                .label = "Visible for registered",
                                .is_default = true},
                               {.type = w_checkbox,
                                .label = "Visible for anonymous",
                                .is_default = false},
                               {.type = w_checkbox,
                                .label = "Visible for my company",
                                .is_default = false},
                               {.type = w_end}};

  w_group_el_init_t actions[] = {
      {.type = w_button, .label = "Upload", .is_default = false, .val.num = 1},
      {.type = w_button, .label = "Cancel", .is_default = false, .val.num = 2},
      {.type = w_end},
  };

  app->modal.w.parent_win = &app->win;
  w_dialogue_init(&(app->modal), "Description for the file",
                "Enter essential data", &(app->coordinates));
  w_dialogue_t *d = &(app->modal);

  d->w.callback = upload_props_dialogue_modal_cb;
  d->g_content = w_group_init(&(d->win), &(d->w), content, &(d->id_map), vertical,
                            g_content);
  d->g_action = w_group_init(&(d->win), &(d->w), actions, &(d->id_map),
                           horizontal, g_action);

  app->query_args->active_dialogue = d;

  w_dialogue_init_active_id(d);

  return NULL;
}