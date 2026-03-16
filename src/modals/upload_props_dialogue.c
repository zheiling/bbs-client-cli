#include "checkbox.h"
#include "group.h"
#include "widget_core.h"
#include <ncursesw/ncurses.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <widget.h>
#include <utils.h>

void upload_props_dialogue_modal_cb(callback_args_t *args) {
  callback_args_t d_args;
  app_t *app = args->app;
  dialogue_t *d = (dialogue_t *)app->active_widget;
  memcpy(&d_args, args, sizeof(callback_args_t));
  d_args.app = NULL;
  d_args.element = app->active_widget;
  dialogue_default_callback(&d_args);
  char query[256];
  int32_t query_len = 0;
  if (d_args.resp_data.code == cbrp_val) {
    checkbox_t *reg_cbx  = (checkbox_t *)d->g_content->elements[0].element;
    checkbox_t *anon_cbx = (checkbox_t *)d->g_content->elements[1].element;
    checkbox_t *comp_cbx = (checkbox_t *)d->g_content->elements[2].element;
    
    int32_t privileges = 0;
    if (anon_cbx->value == true) privileges |= 1;
    if (reg_cbx->value == true)  privileges |= 2;
    if (comp_cbx->value == true) privileges |= 4;

    switch (d_args.resp_data.val.val.num) {
    case 1:
      query_len =
          sprintf(query, "file upload \"%s\" %zu %d\n",
                  app->query_args->file->name, app->query_args->file->size, privileges);
      /* app->query_args->file->description =
          malloc(desc_input->value_len + 9); reserve space for \n:END:\n */
      app->query_args->file->description = malloc(sizeof ("[Empty description]\n:END:\n"));
      strcpy(app->query_args->file->description, "[Empty description]\n:END:\n");
      d->needs_destroy = true;
      app->query_args->state = S_UPLOAD_REQUESTED;
      write(app->query_args->sd, query, query_len);
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

dialogue_t *init_upload_props_dialogue_modal(app_t *app) {
  if (app == NULL)
    return NULL;
  group_el_init_t content[] = {{.type = w_checkbox,
                                .label = "Visible for registered",
                                .is_default = true},
                               {.type = w_checkbox,
                                .label = "Visible for anonymous",
                                .is_default = false},
                               {.type = w_checkbox,
                                .label = "Visible for my company",
                                .is_default = false},
                               {.type = w_end}};

  group_el_init_t actions[] = {
      {.type = w_button, .label = "Upload", .is_default = false, .val.num = 1},
      {.type = w_button, .label = "Cancel", .is_default = false, .val.num = 2},
      {.type = w_end},
  };

  app->modal.w.parent_win = &app->win;
  init_dialogue(&(app->modal), "Description for the file",
                "Enter essential data", &(app->coordinates));
  dialogue_t *d = &(app->modal);

  d->w.callback = upload_props_dialogue_modal_cb;
  d->g_content = init_group(&(d->win), &(d->w), content, &(d->id_map), vertical,
                            g_content);
  d->g_action = init_group(&(d->win), &(d->w), actions, &(d->id_map),
                           horizontal, g_action);

  app->query_args->active_dialogue = d;

  dialogue_init_active_id(d);

  return NULL;
}