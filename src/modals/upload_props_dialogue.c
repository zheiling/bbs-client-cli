#include <widget.h>
#include <ncurses.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void upload_props_dialogue_modal_cb(callback_args_t *args) {
  int32_t response = -1;
  callback_args_t d_args;
  app_t *app = args->app;
  dialogue_t *d = (dialogue_t *)app->active_widget;
  memcpy(&d_args, args, sizeof(callback_args_t));
  d_args.app = NULL;
  d_args.resp_data = &response;
  d_args.element = app->active_widget;
  dialogue_default_callback(&d_args);
  char query[256];
  int32_t query_len = 0;
  input_t *desc_input = (input_t *)d->g_content->elements[0].element;
  if (response > -1) {
    switch (response) {
    case 0:
      query_len =
          sprintf(query, "file upload \"%s\" %zu 1\n",
                  app->query_args->file->name, app->query_args->file->size);
      app->query_args->file->description =
          malloc(desc_input->value_len + 9); /* reserve space for \n:END:\n */
      strcpy(app->query_args->file->description, desc_input->value);
      d->needs_destroy = true;
      app->query_args->state = S_UPLOAD_REQUESTED;
      write(app->query_args->sd, query, query_len);
      break;
    case 1:
      if (app->query_args->file->name != NULL)
        free(app->query_args->file->name);
      if (app->query_args->file->path != NULL)
        free(app->query_args->file->path);
      free(app->query_args->file);
      app->query_args->file = NULL;
      app->query_args->state = S_WAIT_SERVER;
      break;
    }
  }
}

dialogue_t *init_upload_props_dialogue_modal(app_t *app) {
  if (app == NULL)
    return NULL;
  group_el_init_t content[] = {
      {.type = w_input, .label = "File description", .length = 30},
      {.type = w_end}};

  group_el_init_t actions[] = {
      {.type = w_button, .label = "Upload", .is_default = 1},
      {.type = w_button, .label = "Cancel", .is_default = 0},
      {.type = w_end},
  };

  app->modal.w.parent_win = &app->win;
  init_dialogue(&(app->modal), "Description for the file",
                "Enter essential data", &(app->coordinates));
  dialogue_t *d = &(app->modal);

  d->w.callback = upload_props_dialogue_modal_cb;
  d->g_content = init_group(&(d->win), &(d->w), content, horizontal);
  d->g_action = init_group(&(d->win), &(d->w), actions, horizontal);

  app->query_args->active_dialogue = d;

  dialogue_init_active_id(d);

  return NULL;
}