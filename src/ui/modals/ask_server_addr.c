#include "../app.h"
#include "../widget/dialogue.h"
#include "../widget/group.h"
#include <ncurses.h>
#include <string.h>

typedef struct {
  button_t *element;
  enum w_type type;
} group_bt_t;

void asa_modal_group_callback(callback_args_t *args) {
  group_resp_t *response = (group_resp_t *)args->resp_data;
  callback_args_t ch_args;
  memcpy(&ch_args, args, sizeof(callback_args_t));
  ch_args.app = NULL;
  group_default_callback(&ch_args);
  if (response->value > -1) {
    switch (response->value) {
    case 0:
      // exit(10);
    case 1:
      // exit(11);
    case 2:
      // exit(12);
    }
  }
}

dialogue_t *init_asa_modal(app_t *app) {
  if (app == NULL)
    return NULL;
  group_el_init_t content[] = {
      {.type = w_input, .label = "Address", .length = 15},
      {.type = w_input, .label = "Port", .length = 5},
      {.type = w_end}};

  group_el_init_t actions[] = {{.type = w_button, .label = "Connect"},
                               {.type = w_button, .label = "Cancel"},
                               {.type = w_end}};

  init_dialogue(&(app->modal.dialogue), "Connect to server",
                "There is information needed", app->cur_y, app->cur_x);
  dialogue_t *d = &(app->modal.dialogue);

  d->g_content = init_group(&(d->win), &(d->w), content, horizontal);
  d->g_content->w.callback = asa_modal_group_callback;  
  
  d->g_action = init_group(&(d->win), &(d->w), actions, horizontal);
  d->g_action->w.callback = asa_modal_group_callback;

  dialogue_init_active_id(d);

  return NULL;
}